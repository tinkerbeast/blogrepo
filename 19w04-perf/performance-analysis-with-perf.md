
Profiling with `perf`
-------------------------

`perf` is a fairly important tool these days for everything performance related in Linux. This post is going to focus on simple profiling with perf, at most it will be a very informative cheat-sheet. I am focusing on a very specific sub-utility because I generally revisit perf every six months or so with the sole intention of optimising some programme - And it takes quite a bit of time to re-remember everything from the exhaustive documentations. If you want to learn in detail about perf visit the [References](#reference) section for a list of thorough documents.


### <a name="summary"></a>Command summary

Due to the cheat-sheet nature of this post the sequence of commands is given first. The option details and rationale is explained below:

	gcc -fno-omit-frame-pointer -g matrix_mult.c -o mm.out
	sudo perf record -F 99 -g ./mm.out
	sudo perf report --stdio -g folded | head -30
	sudo perf script | gprof2dot --format=perf > mm_profile.dot
	dot -Tsvg mm_profile.dot -o mm_profile.svg && gnome-open mm_profile.svg

### <a name="programme"></a>The reference programme and compilation

I have taken a simple matrix multiply programme here as an example. The reason for this is two folds:

* A matrix multiplication programme has $O(n^3)$ complexity, so it runs for a long time.
* A naive implementation for matrix multiplication has inherent performance issues which can be highlighted.

```c

```

Compile this with your standard `gcc` command:

	gcc -fno-omit-frame-pointer -g matrix_mult.c -o mm.out

Note that while I added debug symbols for the build no extra profiling symbols need to be added. This is perhaps the single-most important aspect of `perf` - *That `perf` is minimally intrusive when it comes to profiling.* The `-fno-omit-frame-pointer` is needed when optimisation options are used for `gcc`. For existing binaries with missing frame pointers, there exists a workaround since Linux 3.9 using the `--call-graph dwarf` when collecting data.


### <a name="perf-record"></a>Recording runtime data

Simply use the [`perf record`][4] subcommand:

	sudo perf record -F 99 -g ./mm.out

Option `-F 99` tells `perf` to record at 99Hz. This means than an interrupt will come and hit the programme every 100ms. It is from this interrupt stack that perf will collect data about the programme. The higher the frequency value, the more intrusive perf becomes.  Assuming most modern OS have a tick time of 10ms, even if the perf interrupt runs for an entire 10ms time slice, you only get a 1% interference to your programme (Assuming your programme runs in reasonably high priority to get most of the CPU time).

Option `-g` tells that the stack frames have to be captured. Otherwise `perf` would collect data per function wise. So if your code has the same function called at different stack-depths, you will be hard pressed to figure out full picture with the perf data collected without stack frames.

If the programme is running for too long, it is perfectly fine to cancel the execution with <kbd>Ctrl+C</kbd>. Stopping the programme or the programme ending by itself should create a file <var>perf.data</var>.

### <a name="perf-analyse"></a>Analyse the data

To analyse the <var>perf.data</var> file we simply use the [`perf report`][5] subcommand:

	sudo perf report --stdio

The `--stdio` option is used to output to the command line rather than opening up an interactive shell. The interactive shell has it's own uses and is discussed in in [Appendix 3: Detailed performance analyisis](#appendix3). For now let us inspect part of the command output:
```
   100.00%     0.00%  mm.out   [unknown]          [.] 0x049e258d4c544155
            |
            ---0x49e258d4c544155
               __libc_start_main
               main
               |          
               |--50.30%--matrix_multiply_wrapper
               |          matrix_multiply
               |          
                --49.70%--matrix_multiply

   100.00%     0.00%  mm.out   libc-2.23.so       [.] __libc_start_main
            |
            ---...
```
An understandably messy but detailed output tells about the call-flow and CPU percentage each function in the flow. While the output is detailed, this will clearly be too confusing for a more practical programme with a complex call flow. If we had run the recording part without the `-g` (stack frame collection option), our output would have been something like:
```
# Overhead  Command  Shared Object      Symbol                   
# ........  .......  .................  .........................
#
    99.62%  mm.out   mm.out             [.] matrix_multiply
     0.11%  mm.out   [kernel.kallsyms]  [k] update_load_avg
     0.07%  mm.out   [kernel.kallsyms]  [k] dequeue_rt_stack
     0.05%  mm.out   [kernel.kallsyms]  [k] __irqentry_text_start
     0.05%  mm.out   [kernel.kallsyms]  [k] native_write_msr
     0.05%  mm.out   [kernel.kallsyms]  [k] perf_event_task_tick
     0.04%  mm.out   mm.out             [.] matrix_init_random
     0.00%  mm.out   [kernel.kallsyms]  [k] mmap_region
     0.00%  perf     [kernel.kallsyms]  [k] native_write_msr

```
A much more comprehensive output immediately highlighting the pain-point function. However in most cases information about the call flow will be fairly necessary.  A similar output can be created from the record data which has the stack frame information using:

	sudo perf report --stdio -g folded

The output for this would be like this:
```
# Children      Self       Samples  Command  Shared Object      Symbol                                        
# ........  ........  ............  .......  .................  ..............................................
#
   100.00%     0.00%             0  mm.out   [unknown]          [.] 0x049e258d4c544155
50.03% 0x49e258d4c544155;__libc_start_main;main;matrix_multiply_wrapper;matrix_multiply
49.54% 0x49e258d4c544155;__libc_start_main;main;matrix_multiply
   100.00%     0.00%             0  mm.out   libc-2.23.so       [.] __libc_start_main
50.03% __libc_start_main;main;matrix_multiply_wrapper;matrix_multiply
49.54% __libc_start_main;main;matrix_multiply
   100.00%     0.00%             0  mm.out   mm.out             [.] main
50.03% main;matrix_multiply_wrapper;matrix_multiply
49.54% main;matrix_multiply
   100.00%    99.58%          1875  mm.out   mm.out             [.] matrix_multiply
50.03% 0x49e258d4c544155;__libc_start_main;main;matrix_multiply_wrapper;matrix_multiply
49.54% 0x49e258d4c544155;__libc_start_main;main;matrix_multiply
    50.30%     0.00%             0  mm.out   mm.out             [.] matrix_multiply_wrapper
50.03% matrix_multiply_wrapper;matrix_multiply
     0.37%     0.16%             3  mm.out   [kernel.kallsyms]  [k] __irqentry_text_start
```
The values in the <var>Self</var> column in the above output is equivalent to the values in the <var>Overhead</var> column in the previous output. So using the `-g` option for collection is always a better choice.

However, for most practical purposes you need to parse the output of the report command for a better visualisation.

### <a name="perf-visualise"></a>Visualise the data

Visualising the `perf` output has been covered well in the Flame Graph section of [Brendan Gregg's perf site](http://www.brendangregg.com/perf.html#FlameGraphs). However the point of view which Flame Graphs cover is that of a system wide analysis. For simple profiling use cases, having a call-graph with usage percentage serves us better. Call graph's can be generated using a myriad of tools - We will be using a good old Graphviz generated graph. Before jumping into the command, note that the [`perf script`][6] subcommand is used to generate detailed trace of the recorded workload. Also [`gprof2dot`](https://pypi.org/project/gprof2dot/) is a fairly popular tool for converting profiler data to call graph.

Install `gprof2dot` using:

	sudo -H pip install gprof2dot

After that generate the Graphviz file using:

	sudo perf script | gprof2dot --format=perf > mm_profile.dot

View the output:

	 dot -Tsvg mm_profile.dot -o mm_profile.svg && gnome-open mm_profile.svg
	 
### <a name="appendix1"></a>Appendix A: Using `perf` on Java programmes

There is a very detailed [presentation by Brendan Gregg](3) for using perf with Java. I highly recommend it because it highlights the nuances with JIT and Java bytecodes which perf does not understand directly. For the sake of brevity, you need:
1. Your `java -version` to be higher than `java version "1.8.0_60"` so that it supports the option `-XX:+PreserveFramePointer`
2. Install the [perf-map-agent from github](https://github.com/jrudolph/perf-map-agent). The installation instructions require your `$JAVA_HOME` to be set appropriately.
3. 

https://www.usenix.org/sites/default/files/conference/protected-files/srecon18americas_slides_goldshtein.pdf
https://medium.com/@maheshsenni/java-performance-profiling-using-flame-graphs-e29238130375

PreserveFramePointer

### <a name="reference"></a>References

[3]: ??? "Java one presentation by "
[4]: ??? "Man page for `perf record`"
[5]: ??? "Man page for `perf report`"
[6]: ??? "Man page for `perf script`"




