History of Linux device management
==================================


Overview
--------

### 1. The Early Era: Static /dev

In early Linux, every possible device existed as a permanent file in the /dev directory, whether the hardware was present or not.

* Mechanics: Administrators created files manually using the mknod command.
* Identification: The kernel mapped hardware using fixed Major (device type) and Minor (instance) numbers.
* The Problem: The system ran out of numbers as hardware expanded. /dev became cluttered with thousands of dead device nodes.

### 2. The First Fix: devfs

Introduced in Linux 2.4, devfs tried to solve the clutter by moving device node creation entirely into kernel space.

* Mechanics: The kernel automatically created nodes inside /dev only when hardware was detected.
* The Problem: It suffered from unfixable race conditions during boot. It also forced naming policies inside kernel space, preventing customization. It was removed in Linux 2.6.18.

### 3. The Userspace Shift: Early udev

Introduced in 2003 (Linux 2.6), udev shifted the responsibility of creating device nodes completely into userspace.

* Mechanics: The kernel detected hardware and sent a uevent via a netlink socket. The udevd daemon caught the event and created the node in a standard tmpfs RAM disk mounted at /dev.
* The Problem: During early boot, the system faced a "chicken-and-egg" dilemma. Userspace (udevd) needed to run to create device nodes (like disk controllers), but the kernel needed those nodes first just to mount the root filesystem and start userspace.

### 4. The Missing Link: devtmpfs

To solve early boot deadlocks, kernel developers introduced devtmpfs in Linux 2.6.32 (2009). This is the foundation of modern Linux booting.

* Mechanics: devtmpfs is a tiny, kernel-managed filesystem mounted at /dev. The moment the kernel detects a core device during boot, the kernel itself instantly creates a basic node (like /dev/sda).
* The Handshake: Once the root filesystem mounts and udevd starts, udevd takes ownership of the devtmpfs mount. It does not replace it; it modifies it.
* The Result: Safe, deadlock-free booting. The kernel handles the raw nodes instantly, while userspace (udev) layers custom names, permissions, and symlinks on top.

### 5. Modern Era: systemd-udevd

In 2012, udev merged into the systemd project as systemd-udevd.

* Role: It manages the devtmpfs filesystem, executing complex rules found in /etc/udev/rules.d/.
* Benefit: Provides persistent naming (e.g., ensuring a USB drive always mounts as backup_disk) and triggers user scripts on hardware changes.

### 6. The Future: eBPF and Virtualization

Device management is evolving to handle cloud containers, virtualization, and speed.

* eBPF Filtering: Systems use eBPF to filter kernel uevents instantly, bypassing traditional userspace processing bottlenecks.
* Namespace Isolation: Modern container runtimes require strict device isolation. Future development focuses on namespace-aware device nodes, allowing containers to safely manage virtual hardware without host intervention.


The Early Era: Static /dev
--------------------------

### 1. Chronology and Evolution of Static /dev

The concept of representing hardware devices as files originates from UNIX (Version 1, 1971) and was adopted by Linux at its inception.

* 1991 (Linux 0.01 to 0.11): Initial Implementation
* Mechanism: Linus Torvalds implemented the basic VFS (Virtual File System) layer mapping file operations (open, read, write) to driver-specific functions based on Major numbers.
   * Reference: Kernel source fs/devices.c in Linux 0.11.
* 1994 (Linux 1.0): Standardization of Major/Minor Numbers
* Mechanism: Major and Minor numbers were strictly bounded as 8-bit integers (limiting the system to 255 device types and 255 instances per type).
   * Reference: The Linux Assigned Names and Numbers Registry (LANAN) was created by H. Peter Anvin to coordinate these fixed mappings.
* 1995 (Linux 1.2): Introduction of the MAKEDEV Script
* Mechanism: Manually typing individual commands became unmanageable. Distributions began shipping the MAKEDEV shell script inside /dev, which contained a hardcoded list of standard system devices.
   * Reference: Historically packaged in standard Linux system base utilities (e.g., MAKEDEV-2.x).
* 2001 (Linux 2.4): The Ceiling and Phase-Out
* Mechanism: Hardware bloat made the 8-bit limit unsustainable. While Linux 2.4 expanded these boundaries internally, it marked the official transition point where static /dev was recognized as obsolete, paving the way for devfs and ultimately udev in Linux 2.6.

### 2. Kernel & Userspace APIs and Mechanisms

In this era, there was zero communication between the kernel and userspace regarding device addition. The file system and the running kernel driver operated completely independently until a userspace application attempted an I/O operation.
```
[ Userspace Application ]
         │
         ▼ (Triggers open("/dev/my_dev"))
   [ VFS Layer ]
         │
         ▼ (Looks up Major Number)
[ Kernel Driver Table ] ────► [ Hardware ]
```

Userspace Mechanism:

* System Call: sys_mknod
* Command: mknod /dev/filename <type> <major> <minor>
* Action: This created an inode directly on the underlying hard drive filesystem (e.g., ext, ext2). The inode stored the device type (block or character) and the Major/Minor pair instead of data block pointers.

Kernel Space Mechanism:

* API (Character Devices): `register_chrdev(unsigned int major, const char *name, struct file_operations *fops)`;
* API (Block Devices): `register_blkdev(unsigned int major, const char *name)`;
* Action: The driver registered itself into a static internal kernel array using a specific Major number.

The Interaction Handshake:

1. An application calls open("/dev/foo", O_RDONLY).
2. The VFS layer parses the inode of /dev/foo, notices it is a character device, and extracts the Major number.
3. The VFS indexes the kernel's internal driver array using that Major number.
4. If a driver is registered at that index, the kernel routes the execution to that specific driver's .open function. If no driver matches, it returns ENODEV (No such device).

> [!IMPORTANT]
> In the early era, /dev existed as actual sectors on your spinning hard drive (formatted with standard filesystems like ext or ext2). If you turned off the computer, the files inside /dev remained saved on the disk. It was mknod that gave special meaning to the files in /dev. Running mknod did not create a normal file that holds data blocks. Instead, it told the filesystem to create a special type of metadata marker called a device inode. When you looked at a file created by mknod, the filesystem stored exactly three critical pieces of information inside that disk entry - a flag stating "This is a device node" (Character or Block), rather than a regular text file, the Major number and the Minor number.

The Key Difference:

* In the Static /dev era, the files were persistent, dead markers sitting on your actual hard drive. The kernel had no idea they existed until you tried to open them.
* In the devfs (and later devtmpfs) era, the /dev directory became a virtual, in-memory filesystem. The files were generated on the fly out of thin air by the kernel and vanished completely the moment you powered down the machine.

### 3. Minimal Example: Device Creation (Linux 1.0 - 2.2 Era)

TODO: See Appendix


Architectural Bottlenecks of Static /dev
----------------------------------------

TODO: This section needs a revisit. 1. Things like boot lag is missed. 2. Need references for all the problems and symptoms state

The static /dev architecture suffered from three core flaws: it lacked hardware awareness, it scaled poorly, and it created severe security risks.

### 1. Total Disconnect Between Hardware and Filesystem
The fundamental flaw was that the filesystem had no communication channel with the kernel's hardware layer.

* The Problem: The files in /dev were just passive markers on a hard drive. If you plugged in a new SCSI tape drive, no file appeared. If you unplugged a disk, its file remained.
* The Symptom: Userspace applications had to blindly guess if hardware existed. A program trying to use a modem had to open /dev/ttyS0, /dev/ttyS1, and /dev/ttyS2 sequentially, waiting for timeout errors (ENODEV) to figure out which port actually had a physical wire attached to it.

### 2. The 8-Bit Numbering Crisis (The Scaling Wall)
The kernel tracked devices using a strict 8-bit Major and 8-bit Minor numbering scheme [1]. This capped the system at 255 device types and 255 individual units per type.

* The Problem: As enterprise servers grew to support hundreds of hard drives (via large storage arrays) or thousands of terminal connections, Linux literally ran out of numbers.
* The Symptom: A standard SCSI disk array easily ate up dozens of minor numbers for partitions (/dev/sda1, /dev/sda2, etc.). There were not enough numbers to represent large-scale hardware configurations.

### 3. Inode Explosion and Disk Performance
Because nobody knew what hardware a user might plug in tomorrow, Linux distributions chose to create every single possible device node ahead of time during installation.

* The Problem: A fresh Linux install in the late 1990s shipped with up to tens of thousands of idle device nodes sitting on the hard drive inside /dev.
* The Symptom: Booting and running a simple file search like find / -name "something" slowed to a crawl because the OS had to sift through thousands of useless device inodes on the disk.

### 4. Hardcoded Permissions and Security Drifts
Because the nodes were static files on a persistent disk, their ownership and permission settings (chmod/chown) were permanently saved to the hard drive.

* The Problem: Dynamic security was impossible. If a specific user logged into a physical terminal and needed access to the audio card (/dev/audio), the system had to globally alter file permissions on the hard disk.
* The Symptom: If the system crashed or power-cycled before resetting those permissions, the audio card remained wide open to unauthorized users on the next boot.


Designing a Solution in the firs era (TODO: Don't like section title)
---------------------------------------------------------------------

If you were a kernel architect in 1999 tasked with solving these flaws without knowing that devfs or udev would be invented, you would have to choose between two fundamentally different solution paths.
```
                    ┌──────────────────────────────┐
                    │  HOW TO AUTOMATE DEVICE NODES│
                    └──────────────┬───────────────┘
                                   │
         ┌─────────────────────────┴─────────────────────────┐
         ▼                                                   ▼
┌─────────────────────────────────┐               ┌─────────────────────────────────┐
│     PATH A: Kernel-Driven       │               │     PATH B: Userspace-Driven    │
│   (In-Memory Virtual Filesystem)│               │      (Event-Driven Daemon)      │
├─────────────────────────────────┤               ├─────────────────────────────────┤
│ • Kernel creates nodes directly │               │ • Kernel sends hardware signals │
│ • Instant, zero-lag boot        │               │ • Daemon reads configuration    │
│ • No userspace tools needed     │               │ • Flexible naming rules         │
└─────────────────────────────────┘               └─────────────────────────────────┘
```

### Path A: The Kernel-Driven Approach (In-Memory Filesystem)

The first conceptual solution is to completely delete /dev from the physical hard disk and turn it into a virtual, real-time window into the kernel's brain.

* The Mechanics: Create a new virtual file system type (similar to /proc). When a driver initializes inside the kernel and detects a piece of hardware, the kernel automatically generates a matching file node in this virtual memory space. When the device is unplugged, the kernel evaporates the file.

Pros:
* Zero Lag: Nodes appear instantly during boot before any userspace programs even load.
* Lightweight: No disk space or inodes are wasted on dead hardware.

Cons:
* Kernel Bloat: Moving file naming policies, permissions, and string formatting inside the kernel core would transfer a lot of responsibility to the kernel. This would cause a cascade effect in the future where more features like security had to be moved inside kernel.
* Rigid Naming: If the kernel decides a disk is named /dev/disc0, the system administrator cannot easily change it to a custom name like /dev/backup_disk.

### Path B: The Userspace-Driven Approach (The Event Bus)

The second conceptual solution leaves the kernel entirely out of the file-creation business. The kernel only handles hardware management and signals userspace when things change.

* The Mechanics: The kernel remains blind to files. Instead, it gets an "Event Broadcaster." Whenever hardware is added or removed, the kernel broadcasts a small message string across a network socket (e.g., "ADD: SCSI_DISK_3"). A background program (a daemon) running in userspace listens to this broadcast, reads a configuration file, and manually calls mknod on a fast RAM disk.

Pros:
* Infinite Flexibility: Userspace tools can run complex scripts, look up database entries, and rename devices dynamically (e.g., naming a network card based on its MAC address).
* Clean Kernel: Keeps policy and string-parsing logic out of the highly protected kernel space.

Cons:
* The Boot Deadlock: To run a userspace daemon, the kernel must first mount the root hard drive. But to mount the root hard drive, the kernel needs the disk controller device node. This creates a circular dependency.


The First Fix: devfs
--------------------

### 1. Design Choices and Solved Problems
When Richard Gooch designed devfs in the late 1990s, he chose Path A: The Kernel-Driven Approach. The core design philosophy was that the kernel is the sole source of truth for hardware, so the kernel should directly manage the device directory.

Core Design Choices:

* Virtual, In-Memory Storage: /dev was detached from the hard drive and turned into a pseudo-filesystem (type devfs). Nodes existed only in RAM.
* Driver-Driven Lifecycle: Device nodes were tightly coupled to driver initialization. When a driver’s init function ran and found hardware, it explicitly registered the device with the devfs subsystem.
* Hierarchical, Rigid Naming: To eliminate naming collisions, devfs abandoned flat names (like /dev/sda1) in favour of strict, hardware-path-based paths (e.g., /dev/scsi/host0/bus0/target0/lun0/part1).

Problems It Fixed:

* Eliminated the /dev Clutter: A fresh boot only displayed files for hardware physically present on the machine.
* Bypassed Major/Minor Resource Exhaustion: Because the kernel looked up devices directly by their virtual filesystem paths/inodes rather than relying strictly on the 8-bit allocation registry, the system could scale past the 255-device limit.
* No More Guesswork for Applications: If /dev/sound/dsp existed, the application knew with 100% certainty that a working sound card was present.

### 2. Chronology and Evolution of devfs

* January 1998 (Linux 2.1.79): The Introduction
* Mechanism: devfs was first integrated into the unstable/development kernel branch as an optional feature.
   * Reference: Kernel patch announcements by Richard Gooch; introduction of <linux/devfs_fs_kernel.h>.
* January 2001 (Linux 2.4.0): Mainstream Adoption
* Mechanism: Linux 2.4.0 shipped with devfs fully integrated into the stable kernel line. While highly controversial, it was heavily adopted by cutting-edge distributions like Gentoo.
   * Reference: Linux 2.4.0 source tree under fs/devfs/.
* December 2003 (Linux 2.6.0): The Beginning of the End
* Mechanism: Linux 2.6 introduced a completely revamped driver model (sysfs and kobject). devfs was marked as deprecated in favour of the newly born udev.
   * Reference: Linux 2.6.0 release notes explicitly advising migration away from devfs.
* June 2006 (Linux 2.6.17 to 2.6.18): Complete Removal
* Mechanism: Greg Kroah-Hartman and other core maintainers completely stripped the devfs source code out of the kernel tree, declaring its architectural flaws unfixable.
   * Reference: Git commit tracking the deletion of fs/devfs/ in the 2.6.18 development cycle.

### 3. Kernel & Userspace APIs and Mechanisms

In the devfs era, the interaction flipped entirely: the kernel was active, and userspace became completely passive.
```
[ Kernel Driver Init ] 
         │
         ▼ (Calls devfs_register())
[ devfs Core Engine ] ──► Automatically populates RAM-based /dev/
         │
         ▼ (Spawns node instantly)
[ Userspace Application ] can now immediately open("/dev/my_device")
```

Kernel Space Mechanism:

Instead of registering a generic Major number and waiting, the driver actively dictated file creation using specific devfs management functions:

* API Handle: devfs_handle_t
* Registration Function: `devfs_handle_t devfs_register(devfs_handle_t dir, const char *name, unsigned int flags, unsigned int major, unsigned int minor, umode_t mode, void *ops, void *info)`
* Deregistration Function: `void devfs_unregister(devfs_handle_t handle)`


Userspace Mechanism:

* No mknod Required: Userspace utilities did not need to run commands to create device paths.
* Mounting: The system administrator or init scripts simply mounted the filesystem via /etc/fstab or early boot commands: `mount -t devfs none /dev`
* The Daemon Helper (devfsd): Because names were now hardcoded by the kernel (e.g., /dev/sound/dsp), an optional userspace daemon called devfsd was introduced. It listened to internal devfs changes to create backwards-compatible symlinks (like making /dev/dsp point to /dev/sound/dsp) and manage dynamic file permissions.

> [!IMPORTANT]
> In the previous era there was a disconnect between the user and kernel space. But not a disconnect with physical device and device files in /dev since all possible devices were created. However with this new mechanism of on demand device node creation we have to associate the physical devices with the OS. Two mechanisms were used in this era -
> The Hardcoded Standard:
> The distribution was maintainers didn't compile drivers as separate modules (.o or .ko files). Instead, they built a monolithic kernel. You compiled every driver you thought you might ever need directly into the main kernel binary (`vmlinuz`). On boot, the kernel would run through every single built-in driver sequence. The IDE disk driver would probe standard motherboard addresses, the floppy driver would poke the floppy controller, and the sound driver would blast a signal to a hardcoded ISA slot address to see if a SoundBlaster card responded. If nothing was there, the driver sat idle in memory.
> Userspace Injections: `insmod` / `modprobe` in Init Scripts:
> If a driver was compiled as a module, it was up to the userspace system initialization scripts (the SysV init scripts under /etc/rc.d/) to manually force-load them using insmod or modprobe. The system administrator had to manually edit a configuration file—most notably /etc/modules.conf (later /etc/modprobe.conf). You had to explicitly map hardware types to module names:
> 	alias eth0 3c59x      # Tells the system: if something asks for network 'eth0', load the 3Com driver
> 	alias sound-slot-0 sb # Tells the system: for audio, load the SoundBlaster module
> 	options sb io=0x220 irq=5 dma=1 # Manually assigning physical hardware resources
> The boot scripts read this file line-by-line and executed modprobe module_name sequentially, assuming the hardware was actually present.

### 4. Minimal Example: Device Creation (Linux 2.4 Era)

TODO: See Appendix.


Revamped driver model (kobject and sysfs) and devfs shortcomings
----------------------------------------------------------------

### 1. Why the Driver Model Was Revamped
Prior to Linux 2.6, the kernel lacked a unified representation of the machine's physical hardware layout. It suffered from two fatal structural blind spots:

* No Unified Topology (The Parent-Child Problem): The kernel didn't understand device relationships. It didn't know that a USB flash drive was plugged into a USB Hub, which was plugged into a PCI USB Controller, which sat on the PCI Bus. If the system went to sleep, the kernel might power down the PCI Controller before flushing data on the flash drive, causing catastrophic data corruption.
* Decentralised Driver Registries: Every subsystem (PCI, USB, SCSI, ISA) managed its own internal arrays, tracking mechanisms, and probing code. There was massive code duplication across the kernel, making it impossible to enforce system-wide power management or discovery standards.

### 2. The New Driver Model: kobject and sysfs
Linux 2.6 introduced a unified object-oriented framework (written in C) to represent every single component of the operating system. At this point in history, devfs was still active and managing /dev, but the underlying engine tracking the hardware was completely replaced.
```
                      [ Physical Core Hardware ]
                                  │
                 (Engineered as a hierarchy of kobjects)
                                  ▼
                        [ Unified Driver Model ]
                       ┌────────────────────────┐
                       │  kobject / kset Engine │
                       └───────────┬────────────┘
                                   │
                                   ▼ (Real-time Memory Map)
                       ┌────────────────────────┐
                       │         sysfs          │
                       ├────────────────────────┤
                       │ • Mounted at /sys      │
                       │ • Folders = kobjects   │
                       │ • Files = Attributes   │
                       └────────────────────────┘
```
#### The kobject (Kernel Object)

The kobject is the foundational structure of the modern Linux driver model. It is embedded inside higher-level structures (like struct device or struct driver). Its design directly solved the problems of the previous era:

* Parent-Child Hierarchies: Every kobject has a pointer to a .parent kobject. This links devices into a true hierarchical topology tree mirroring physical buses. Because the kernel now had a clear graph of the hardware, it could traverse the tree from the bottom up to safely suspend devices (e.g., disabling the flash drive before the PCI controller).
* Reference Counting: To manage dynamic hardware safely, kobject tracks exactly how many components of the kernel are interacting with an object via kobject_get() and kobject_put(). A device structure cannot be freed or erased from kernel memory if a process, driver, or virtual file is still accessing it.
* Grouping (kset): Combines similar kobjects together (e.g., all network cards are grouped into a kset for network devices), standardising driver code across different buses.

#### The sysfs Filesystem

sysfs is an in-memory virtual filesystem (mounted at /sys) that serves as a direct visual mirror of the kernel's internal kobject tree.

* Folders: Every kobject created inside the kernel automatically materialises as a directory in /sys.
* Files (Attributes): The structural properties of a kobject (like a disk's size or a device's power state) are exported as standard read/write text files. Reading a file inside /sys runs a small kernel function that fetches live metrics directly from the physical silicon, eliminating decentralised configuration tracking.


### 3. How It Interacted with devfs

The introduction of the new driver model alongside devfs in early Linux 2.6 releases created massive architectural friction, resulting in deep redundancies and fatal structural incompatibilities due to conflicting concurrency designs.

#### Redundancy

The kernel suddenly had two separate filesystems attempting to represent the same hardware. A driver had to register with the new driver model to map its power topology and hardware parameters in sysfs (/sys), and then turn around and call devfs_register() to expose its actual functional file node in userspace (/dev). The kernel was weighed down by dual-maintenance code for two entirely different virtual file trees.

TODO: Small example would be good here

#### The Concurrency Clash: Synchronous vs. Asynchronous Architecture

The fatal flaw lay in how both systems handled time and state changes.

* devfs was Synchronous and Blocking: It was designed under the assumption that device operations happened sequentially on a single execution thread. When a device node was being opened or closed, devfs held global internal locks. It assumed that hardware wouldn't vanish while a thread was executing code inside its file operations.
* The New Driver Model was Asynchronous and Event-Driven: To support hotpluggable buses (like USB and FireWire) without freezing the entire operating system, the Linux 2.6 driver model moved hardware management to asynchronous execution paths. Hardware discovery, probing, and teardown routines were pushed to separate, non-blocking kernel threads (managed by workqueues).

This architectural mismatch introduced critical, unfixable race conditions when handling hardware removal:
```
[ Kernel Worker Thread ]                  [ Userspace Application Thread ]
           │                                             │
 (USB Device Unplugged)                                  │
           │                                             │
   Dismantles kobject                                    │
   & Frees Driver Memory                                 │
           │                                  Calls open("/dev/usb_dev")
           ▼                                             │
[ Dead Memory Pointer ] <──────────────────────── Is Routed by devfs
           │                                             │
           X ───► CRASH: Null Pointer Dereference ◄──────┘
```

1. The Trigger: A user abruptly unplugged a busy USB device.
2. The Asynchronous Teardown: The physical bus controller generated a hardware interrupt. The modern driver model caught this interrupt and immediately dispatched a background kernel worker thread to gracefully dismantle the device's kobject tree and free the driver's backing memory structures.
3. The Userspace Interleaving: At the exact same millisecond, a userspace application thread executed a system call (like open(), read(), or close()) on the device node inside /dev.
4. The Collapse: Because devfs operated on a completely separate, isolated code path, it was entirely blind to the asynchronous worker thread dismantling the hardware core. It did not participate in the kobject reference counting system (kobject_get/kobject_put).
5. The Crash: devfs happily allowed the userspace thread to proceed, routing its execution straight to the memory address of the driver's file operations structure. Since the asynchronous thread had already cleared and freed that exact memory segment, the kernel attempted to execute garbage code, resulting in an immediate system-wide crash (Kernel Panic via Null Pointer Dereference).


### 4. Shortcomings of devfs Overcome by the New Model

By moving to the kobject and sysfs design, Linux successfully resolved the exact structural limitations that crippled devfs:

| Feature | The devfs Defect (Path A) | The kobject/sysfs Solution |
|---|---|---|
| Naming Authority | Hardcoded by the kernel driver developer. If a developer named a disk /dev/ide/host0/..., userspace was stuck with it. | Separation of Policy: sysfs only shows the raw connection path (e.g., /sys/bus/pci/...). It makes no naming assumptions, which laid the groundwork to let userspace handle naming later. |
| Memory Footprint | Wasted massive kernel memory storing custom string paths, directory structures, and metadata definitions for every file node. | Highly Optimized Core: A kobject uses simple pointer structures. It completely divorced the core memory representation of hardware from the file-handling logic. |
| Hotplug Signaling | Clarification below: devfs provided no native broadcast mechanism to notify userspace when a device appeared or vanished. | Foundation for Broadcasters: While sysfs itself is a passive filesystem, the underlying kobject framework was designed to easily trigger notifications whenever an object state changed. |

### Clarification on devfs Hotplug Capabilities
To clarify, devfs itself did not support hotplug broadcasting. It was a purely internal kernel-space filesystem. When a device was plugged in, devfs simply mutated its own internal RAM-disk tree. If userspace needed to react to that change (such as running a script to set audio volumes when a sound card appeared), it could not find out from devfs.
Instead, early systems had to rely on a separate, crude kernel mechanism called the /proc/sys/kernel/hotplug multiplexer. This older multiplexer fork-executed a brand new shell script process (usually /sbin/hotplug) out of thin air for every single hardware interrupt. This was incredibly slow and heavy, as spawning thousands of shell processes during a fast boot cycle exhausted system PIDs and stalled execution. The kobject subsystem completely replaced this sluggish design by laying the foundation for a lightweight, socket-based event stream.
If you are ready to see how this architecture finally pushed developers to get rid of devfs entirely, we can break down:


### 5. Other devfs issues

#### The Early-Boot Deadlock (The Naming Catch-22)
As Linux distributions grew more complex, they began using modular kernels where crucial storage drivers (like SCSI, RAID, or specialized file systems) were compiled as external modules stored on the hard drive rather than being baked into the monolithic kernel image. This triggered a critical boot deadlock in devfs:

* The Circular Dependency: To mount the root filesystem (/), the kernel needed to load the storage driver module from the disk. To load that module, userspace scripts had to run. But to run userspace scripts, the kernel first needed to mount the root filesystem.
* The devfsd Reliance: Because devfs used radical, long, hardcoded hardware paths (e.g., /dev/scsi/host0/bus0/target0/lun0/part1), older userspace software and boot scripts completely broke down. They expected traditional names like /dev/sda1. To bridge this gap, systems relied on the userspace daemon devfsd to run early in the boot cycle and create backwards-compatible symlinks.
* The Standstill: If devfsd was required to create the legible symlinks that the boot scripts needed to find and mount the root disk, but devfsd couldn't be loaded because the root disk wasn't mounted yet, the system hit an unrecoverable early-boot deadlock.

#### Massive, Unjustified Kernel Memory Footprint
Because devfs was built inside kernel space, every single file node, directory string, permission flag, and symlink layout lived directly in the kernel's highly protected, non-swappable RAM allocations.

* The Issue: On high-end enterprise servers of the era featuring hundreds of storage volumes, tape drives, and partitions, devfs consumed megabytes of critical kernel memory just to store string paths and VFS metadata structures.
* The Inefficiency: This completely violated kernel design paradigms. The core kernel memory should be reserved for critical execution logic and hardware state tracking, not for caching text strings of file names.

#### Absolute Violation of the Unix Philosophy (Policy in Kernel)
A foundational rule of operating system design is the Separation of Mechanism and Policy. The kernel should provide the mechanism (how to read the hardware), while userspace should dictate the policy (what the device is named and who can touch it).

* The Issue: devfs forced naming policy directly into the kernel source code. If a system administrator wanted to rename a specific device node for an application requirement, it was impossible without modifying, re-compiling, and reloading the kernel driver itself.
* Implicit Permissions: Dynamic permission changes (like giving a specific logged-in user access to the sound card) could not be handled natively by the kernel. Trying to hack this functionality into the kernel via userspace helper hooks caused massive synchronization bugs.

#### Code Maintenance Nightmare and High Complexity
Because devfs bypassed the standard Virtual File System (VFS) layers to create files dynamically out of thin air, it required deep, invasive hooks inside the core kernel code.

* The Issue: Every single driver developer in the Linux ecosystem had to write and maintain complex, verbose devfs registration blocks inside their code.
* The Rejection: The code became incredibly messy, brittle, and difficult to audit for security bugs. Linus Torvalds and the core filesystem maintainers openly detested the code quality of the devfs subsystem, describing it as an unmaintainable hack that bypasses proper VFS abstractions.


The Userspace Shift: Early udev
-------------------------------

### 1. The Architecture of Early udev

Introduced by Greg Kroah-Hartman in 2003 (Linux 2.6), udev shifted the responsibility of creating device files out of the kernel space and into the userspace. This decoupled the physical registration of hardware from the presentation of device entries to the user.

To fix the disconnect, let's trace the architecture as a continuous chain of events. In early udev, the creation of a device file was a multi-stage relay race where each component directly triggered the next.
Here is how the execution flowed from physical silicon to a file inside /dev:
```
[ 1. Physical Hardware ] ──(Voltage shift)──► [ 2. Kernel Core / Bus Driver ]
                                                      │
                                             (Invokes uevent logic)
                                                      ▼
[ 4. Userspace: udevd ] ◄──(Pushed via Netlink)── [ 3. kobject Subsystem ]
         │
  (Reads /sys & rules)
         ▼
[ 5. RAM-Disk (tmpfs) ] ──(Runs mknod)──► [ 6. Legible Device Node ]
```

* Step 1: The Physical Hardware Shift: A hardware state change occurs (e.g., you plug in a USB mouse, causing a physical voltage change on the motherboard's USB controller bus). This hardware interrupt wakes up the kernel.
* Step 2: The Kernel Core and Bus Driver Layer: The low-level subsystem driver (like usb-storage or pci) catches the interrupt. It reads the raw hardware registers to confirm a new device exists. To register this device internally, it passes the data up by initializing a new tracking structure in the kernel's object subsystem.
* Step 3: The kobject Subsystem Packaging: The kernel wraps this device inside a standard tracking structure called a kobject. The kobject immediately maps a corresponding read-only directory inside /sys. Once mapped, the subsystem code generates an environment text block containing raw parameters (like Major and Minor numbers) and bounces it across the boundary into userspace by writing it directly into an open Netlink network socket.
> [!IMPORTANT]
> With devfs gone, /dev was now a tmpfs mount. When the computer booted, the old static directory on your hard drive was completely hidden. Very early in the boot sequence, a startup script or the init process executed a standard mount command `mount -t tmpfs none /dev`. Right after mounting, /dev was a completely empty directory sitting in your system's volatile RAM. It did not contain a single file.
* Step 4: The Userspace udevd Daemon Catch: A persistent background daemon named udevd is running a non-blocking loop in userspace, listening specifically to that Netlink socket. The moment the text block arrives, udevd wakes up. It reads the data, parses the unique path of the device inside /sys to look up deeper hardware attributes, and matches those attributes against the system's text rules database (/etc/udev/rules.d/).
* Step 5: The RAM-Disk (tmpfs) Execution: Once udevd calculates the correct name, owner, and permission parameters based on its rules, it interacts directly with the in-memory RAM filesystem mounted at /dev (which is a standard tmpfs file system).
* Step 6: The Final Device Node Creation: Inside that tmpfs structure, udevd issues a standard system call (sys_mknod). This creates the actual device node (e.g., /dev/mouse0). The process is complete, and userspace applications can now safely open the file to read the hardware.


### 2. Chronology and Evolution of Early udev

* April 2003 (Linux 2.5.68): The Proof of Concept
* Mechanism: Greg Kroah-Hartman released the first udev prototype. It ran as a simple user utility triggered by the old, blocking /sbin/hotplug kernel intercept hook.
   * Reference: Greg Kroah-Hartman's foundational whitepaper: "udev - A Userspace Implementation of devfs".
* December 2003 (Linux 2.6.0): Stable Integration
* Mechanism: The stable Linux 2.6.0 kernel introduced sysfs and kobjects. This provided the unified, read-only hardware data structure that udev needed to map and replace devfs.
   * Reference: Linux 2.6.0 core kernel source tree under lib/kobject.c.
* December 2004 (Linux 2.6.10): The Netlink Socket Leap
* Mechanism: The kernel received the NETLINK_KOBJECT_UEVENT update. This abandoned the heavy, process-forking /sbin/hotplug design for a fast streaming socket that piped events directly into udevd.
   * Reference: Linux 2.6.10 network subsystem patch logs introducing kernel-to-userspace netlink multicast groups.
* June 2006 (Linux 2.6.18): Complete Elimination of devfs
* Mechanism: With udev paired with early-boot initramfs wrappers across major distributions, the obsolete devfs source code was completely stripped out of the kernel.
   * Reference: Mainline Linux kernel Git commit: "Remove devfs from the kernel tree".


### 3. The Birth of the uevent Netlink Socket
To pass data across the strict boundary separating kernel space from userspace without stalling performance, Linux developers had to abandon the old, heavy hotplug architecture. In Linux 2.0 through early 2.4, whenever a device was plugged in, the kernel ran a blocking executive function that explicitly fork-spawned an independent, short-lived shell process out of thin air (usually /sbin/hotplug). If you plugged in a complex device that triggered 50 internal sub-component adjustments, the kernel was forced to spawn 50 heavy system PIDs sequentially. During a fast boot cycle, this process exhaustion stalled the CPU.
To fix this bottleneck, Linux 2.6.10 introduced a streaming network architecture for hardware changes: the NETLINK_KOBJECT_UEVENT socket interface. Instead of spawning processes, the kernel was given an internal network card that pipes binary text strings directly to userspace via an in-memory socket interface.

#### The Deep Architecture of the Kernel-to-Userspace Bounce

The event transmission executes as an asynchronous, non-blocking sequence across four distinct stages:
```
[ Kernel Workspace: Workqueue Thread ]
                  │
                  ▼ (1. Triggers Device Register)
     [ kobject_uevent_env() ]
                  │
                  ▼ (2. Allocates socket memory sk_buff)
     [ netlink_broadcast() ] ──(3. Pushes text strings)──► [ Socket Queue Buffer ]
                                                                     │
 ═══════════════════════════════════════════════════════════════════╪════ BOUNDARY
                                                                     ▼
                                                           [ Netlink Socket FD ]
                                                                     │
                                                    (4. recvmsg() wakes up daemon)
                                                                     ▼
                                                             [ Userspace: udevd ]

```
1. The Trigger: A background kernel worker thread (managed via system workqueues) registers a hardware change and invokes kobject_uevent(&dev->kobj, KOBJ_ADD).
2. The Packaging: The execution transitions to kobject_uevent_env(). This function allocates a network socket memory buffer (called an sk_buff). It populates this raw byte block with an array of sequential, null-terminated environment text strings containing basic hardware identifiers.
3. The Blast: The kernel calls netlink_broadcast(). This copies the completed byte payload straight into the queue buffer of the NETLINK_KOBJECT_UEVENT multicast group. The kernel thread instantly drops the packet into memory and returns to processing hardware lines. It never waits or blocks for userspace to read the message.
4. The Catch: On the other side of the boundary, the udevd daemon has opened a standard network socket file descriptor bound to that same multicast group. It sits inside a persistent, non-blocking monitoring loop using the poll() or select() system calls. The absolute millisecond that the kernel dumps the byte payload into the queue, the socket file descriptor signals readability. udevd wakes up instantly, executes a standard network data read call (recvmsg()), and pulls the raw string array directly into its own memory space for evaluation.

#### Decoding the Raw String Payload

The byte payload extracted by udevd from the socket is a dense collection of simple key-value configuration strings. It contains no human-readable text logic—just raw parameters detailing what happened and where to inspect the system core for details:
```
ACTION=add
DEVPATH=/devices/pci0000:00/0000:00:1d.7/usb1/1-1/1-1:1.0
SUBSYSTEM=usb
MAJOR=180
MINOR=0
SEQNUM=1043
```
* ACTION: Tells userspace the lifecycle change (add, remove, or change).
* DEVPATH: The exact folder path inside the memory-based /sys tree where this specific item is registered.
* SUBSYSTEM: The core kernel classification engine responsible for managing this hardware.
* MAJOR / MINOR: The physical driver table identification coordinates used to map userspace system calls to device drivers.
* SEQNUM: A monotonically increasing integer used by userspace to reorder packets if network threads process them out of chronological order.


### 4. How Early udev Read /sys to Replace devfs
The raw string packet delivered via the Netlink socket contains minimal metadata (such as the base paths, device type, and assigned node tracking numbers). To assign customized names and specific system access rules, udevd relied on the unified interface introduced in Linux 2.5/2.6: the sysfs filesystem mounted at /sys.
```
[ Incoming uevent Packet ] ──► Extracted: DEVPATH="/devices/pci0000:00/.../1-1"
                                      │
                                      ▼
[ udevd Resolves Path ]    ──► Maps to: /sys/devices/pci0000:00/.../1-1/
                                      │
                                      ▼
                               Scrapes local attributes:
                               ├── /sys/.../1-1/idVendor (Reads "045e")
                               └── /sys/.../1-1/serial   (Reads "9C22B")

```
1. Path Resolution: When a packet was received, udevd extracted the DEVPATH string and combined it with the root /sys directory to find the live location of the hardware data in memory.
2. Attribute Scraping: udevd opened and read the files inside that specific directory path. Reading these virtual files executes short kernel functions that return live information like manufacturer IDs (idVendor) or hardware sequence markers (serial).
3. Rule Mapping & Node Population: udevd processed these details through configuration files inside /etc/udev/rules.d/. If an entry matched the verified attributes, udevd overrode generic labels and generated the named node:
```   
// mknod(path, mode | block_type, makedev(major, minor))
mknod("/dev/custom_mouse", S_IFCHR | 0660, makedev(180, 0));
``` 

### 5. How Early udev Solved devfs Problems
By converting device node generation into a purely userspace operation, early udev fundamentally dismantled the structural shortcomings that broke devfs.
### Detailed Review: The Concurrency Clash
devfs was isolated from the kernel's core driver lifecycle. When an application opened a device file, devfs routed the operations to the driver's memory structure but did not track the hardware's physical connection state. If a user ripped out a USB device while an application was actively calling read() or write(), a kernel race condition unfolded: the kernel's hotplug thread freed the driver memory, while the application thread tried to execute code at that old address, forcing a system-wide Kernel Panic via Null Pointer Dereference.
udev completely neutralized this clash by pairing with the unified driver model's object reference counting (kobject_get/kobject_put). The file operations are no longer linked directly to loose file paths. Instead, the userspace file descriptor pins the core kernel device structure into RAM.
```
[ devfs: Crash Paradigm ]
Userspace Application ──► Holds File Node Descriptor ──► Points to Memory
                                                            │ (Unplug Event)
Kernel Worker Thread  ──────────────────────────────────────┼──► Frees Driver Memory
                                                            ▼
                                                   [ KERNEL PANIC: Invalid Pointer ]

[ udev: Safe Paradigm ]
Userspace Application ──► Holds File Node Descriptor ──► Points to Memory (Ref Count +1)
                                                            │ (Unplug Event)
Kernel Worker Thread  ──► Marks kobject Disconnected ──────┼──► Safe: Memory Kept Live
                                                            ▼
                                                   [ Application receives -ENODEV ]

```
1. When a device is detached, the kernel marks the device structure as physically disconnected (dead), but it does not free the memory structure.
2. The kernel safely fires a remove packet via the Netlink socket to udevd, which deletes the file from the /dev RAM disk.
3. If the application is still actively holding the file descriptor and tries to issue an I/O operation, the kernel reads the dead status flag and gracefully terminates the application system call with an I/O error code (-ENODEV).
4. Only when the application finally closes its file handle does the reference count drop to zero, allowing the kernel to safely free the driver memory space.

### The Early-Boot Deadlock

* The devfs Problem: Storage drivers lived as external modules on the unmounted hard drive. To parse the disk, the kernel needed standard names like /dev/sda1, but devfs exported nested paths like /dev/scsi/.... This required the userspace daemon devfsd to run early and create familiar symlinks, causing a boot deadlock: the system could not mount the disk without the symlinks, but it could not run devfsd because the binary sat on the unmounted disk.
* The udev Outcome (Unsolved Natively): Early udev did not solve this deadlock natively within the kernel code. Because udevd was a pure userspace program, it suffered from the exact same "chicken-and-egg" bootstrap problem.
* The Userspace Workaround: The Linux community engineered a major architecture patch around udev called initramfs (Initial RAM Filesystem). A tiny root filesystem containing a standalone copy of udevd, a minimal ruleset, and essential disk modules was packed into a compressed archive. The bootloader loaded this entire archive straight into physical RAM during power-on. The kernel booted using this RAM disk as its temporary root directory, enabling udevd to execute instantly in memory, discover the storage hardware, generate the required /dev/sda entries, mount the real physical disk, and gracefully hand off control to the primary OS initialization sequence.

### Massive Kernel Memory Footprint

* The devfs Problem: Every single directory level, device node marker, and permission string was allocated directly inside the kernel's heavily protected, non-swappable RAM tables, swallowing megabytes of critical memory on enterprise servers.
* The udev Solution: udev shifted its entire device node tree into a standard tmpfs RAM disk mounted at /dev. This transferred the naming overhead completely out of protected kernel space and into standard, virtual userspace memory pages that can be safely paged out during an extreme memory crunch.

### Absolute Violation of the Unix Philosophy

* The devfs Problem: Naming policy was hardcoded directly inside the C source code of the kernel drivers. If an administrator wanted to change a node name or enforce a specific user permission constraint, it was impossible without modifying, compiling, and loading a custom kernel driver.
* The udev Solution: udev achieved a clean Separation of Mechanism and Policy. The kernel remains a pure hardware mechanism that tracks entries in /sys and broadcasts raw telemetry packets over Netlink. Naming authority and permission layouts (policy) were entirely shifted to standard plain-text files inside /etc/udev/rules.d/.

## Code Maintenance Nightmare and High Complexity

* The devfs Problem: Because devfs bypassed standard Virtual File System (VFS) layers to create and destroy files on the fly, driver developers had to manually hardcode highly specific, brittle, and verbose devfs manipulation blocks inside every single hardware driver in the Linux codebase.
* The udev Solution: The invasive code blocks were entirely stripped out of the global kernel tree. Driver developers only had to write standard code to register their hardware to the core bus subsystems (PCI, USB) using the clean sysfs driver API framework. udev absorbed the massive complexity of parsing rules, generating filenames, and enforcing directories, restoring code readability and making the kernel highly maintainable.



Practical device drivers in udev
--------------------------------

### 1. The Automated Minimal Driver Tree (tiny_dev.c)
This C codebase represents a minimalist platform module wrapper. It includes the structural macros (MODULE_DEVICE_TABLE) required to announce supported hardware signatures to userspace, which allows udev to handle automatic driver injection via modprobe.
```
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mod_devicetable.h>

MODULE_LICENSE("GPL");
static int major;
static struct class *cls;

/* 1. Define a list of virtual hardware IDs this driver claims to support */
static const struct platform_device_id tiny_id_table[] = {
    { "tiny_silicon_v1", 0 },
    { "tiny_silicon_v2", 1 },
    { } /* Null terminator required */
};
/* 2. Export the table. The build system scrapes this to generate the MODALIAS strings */
MODULE_DEVICE_TABLE(platform, tiny_id_table);

static int __init tiny_init(void) {
    // Snag a character device major index dynamically
    major = register_chrdev(0, "tiny_dev", &NULL_fops);
    
    // Spin up a visibility structure in /sys/class/tiny_cls/
    cls = class_create(THIS_MODULE, "tiny_cls");
    
    // Spawns /dev/tiny_dev in devtmpfs AND broadcasts the netlink packet!
    device_create(cls, NULL, MKDEV(major, 0), NULL, "tiny_dev");
    return 0;
}

static void __exit tiny_exit(void) {
    device_destroy(cls, MKDEV(major, 0)); // Broadcasts remove packet
    class_destroy(cls);
    unregister_chrdev(major, "tiny_dev");
}

module_init(tiny_init);
module_exit(tiny_exit);
```
#### What Happens Under the Hood: The Synchronous-to-Asynchronous Relay
When this code loads into memory, a tightly coordinated chain reaction bridges kernel data storage, virtual file generation, and userspace file manipulation:

1. The kobject Inception: The call to class_create instantly instantiates a parent kobject tracking structure inside the kernel memory. When device_create is invoked immediately after, it creates a child kobject wrapped inside a struct device. This child kobject automatically references the class kobject as its logical parent, building a hardware hierarchy tree.
2. The sysfs Mirror Mutation: The creation of these kobjects triggers the Virtual File System (VFS) driver model. sysfs reads the memory layouts and materializes corresponding, nested directory folders in RAM.
	* class_create forces a directory to appear at: /sys/class/tiny_cls/
	* device_create nests a device entry folder directly underneath it at: /sys/class/tiny_cls/tiny_dev/
3. Attribute Generation: Within /sys/class/tiny_cls/tiny_dev/, the kernel populates standard text files called attributes (such as dev containing the raw major:minor coordinates, uevent, and subsystem).
4. The devtmpfs Population: Simultaneously, because device_create was invoked, the kernel-mediated devtmpfs subsystem instantly reads the raw major:minor values from the freshly generated dev attribute file. It executes an internal, kernel-space sys_mknod to spawn a raw, unconfigured character device file at /dev/tiny_dev.
5. The Netlink Broadcast: Only after the sysfs tracks are mapped and the raw devtmpfs node is generated does the kernel pull the metadata variables from /sys/class/tiny_cls/tiny_dev/uevent and blast an asynchronous packet containing the DEVPATH, MAJOR, MINOR, and MODALIAS strings over the Netlink socket.
6. The Userspace Handshake: The udevd daemon catches this Netlink broadcast. It extracts the DEVPATH, resolves the folder back to /sys/class/tiny_cls/tiny_dev/, parses your custom text files inside /etc/udev/rules.d/, and takes direct ownership of the pre-existing /dev/tiny_dev node—overriding its group ownership, modifying its file permissions, and creating your custom userspace symlinks in a single unified sweep.


### 2. The Complete Userspace Policy Bundle
Once the kernel module executes device_create, udev steps in to coordinate module loading, manage node permissions, and execute external scripts.

#### Step A: The Automatic Module Mapping Line (modules.alias)
When you run make modules_install or execute depmod, the system scans your embedded MODULE_DEVICE_TABLE macro. It automatically generates a mapping file inside the file system:

* Target File Path: `/lib/modules/$(uname -r)/modules.alias`
* Generated Mapping Entry:
```
alias platform:tiny_silicon_v1 tiny_dev
alias platform:tiny_silicon_v2 tiny_dev
```

When a device matching that hardware configuration is discovered, the kernel fires an asynchronous Netlink packet containing a MODALIAS=platform:tiny_silicon_v1 attribute string. A global default udev rule catches this string and forwards it to modprobe, which looks up this modules.alias map file and injects tiny_dev.ko automatically without human intervention.

#### Step B: Custom Filter Matrix Rules (/etc/udev/rules.d/99-tiny.rules)
By default, devtmpfs locks new nodes to root:root with a restricted 0600 bitmask. This rule captures the device class, scales execution permission targets, builds a clean symlink, and hooks an external helper utility.
```
# Match the core kernel properties and manipulate node parameters
SUBSYSTEM=="tiny_cls", KERNEL=="tiny_dev", \
    GROUP="video", MODE="0660", \
    SYMLINK+="hardware/core_bus", \
    RUN+="/usr/bin/tiny_helper.sh"
```
To understand the precise matching keys, string syntax rules, and operators used here, read the official [man 7 udev Reference Manual](https://man.archlinux.org/man/udev.7.en) or explore Daniel Drake's detailed guide on [Writing udev rules](https://reactivated.net/writing_udev_rules.html).
#### Step C: Static Indexing Tables (/etc/udev/hwdb.d/99-tiny.hwdb)
When handling hundreds of hardware iterations using the same base driver core, parsing sequential text rules degrades performance. Instead, we deploy properties using the fast hardware database.
```
# Match the unique Modalias signature pattern of the sub-assembly line
platform:tiny_silicon_v1*
 ID_VENDOR_NAME=CustomSiliconCorp
 ID_HARDWARE_REVISION=4
```
Once customized, compile this text ledger into a high-speed binary search trie by running sudo udevadm hwdb --update. Learn more about system indexing structures via the systemd-hwdb documentation.
#### Step D: Operational State Orchestration (/usr/bin/tiny_helper.sh)
Drivers shouldn't handle initial operational parameters. Instead, our rule catches the insertion event and uses this external, non-blocking handler to log events or write configuration constants straight back into /sys/.
```
#!/bin/sh# Executed by udevd asynchronously the instant the node settles inside /dev
LOG_TARGET="/var/log/tiny_hw.log"

echo "Hardware initialized by udev at $(date)" >> "$LOG_TARGET"
```

### 3. Verification and Lifecycle Toolkit
To troubleshoot, inspect, and test your complete driver deployment pipeline, leverage these essential userspace utilities:

* udevadm monitor: Hooks into the Netlink loop to print real-time events. Running udevadm monitor --environment lets you inspect the raw MODALIAS, MAJOR, and DEVPATH variables passed by your module.
* udevadm info: Queries the internal database cache. Run udevadm info --query=all --name=/dev/tiny_dev to verify that your rules file, custom symlinks, and hwdb parameters were applied correctly.
* udevadm test: Simulates a device event without cycling physical hardware loops or reloading kernel space code. Running udevadm test /sys/class/tiny_cls/tiny_dev walks through your files line-by-line, showing you exactly which filter rules matched and why.

For a comprehensive breakdown of debugging arguments and diagnostic parameters, consult the online [udevadm(8) manual page](https://linux.die.net/man/8/udev).


TODO: Brief history of initramfs
--------------------------------

TODO: The Missing Link: devtmpfs
--------------------------------

TODO: device tee blobs what they changed
----------------------------------------

TODO: Not so brief history of init: System V era
------------------------------------------------

TODO: Not so brief history of init: Systemd era
-----------------------------------------------

TODO: Modern Era: systemd-udevd
-------------------------------

TODO: The Future: eBPF and Virtualization
-----------------------------------------




Appendix
========

TODO
----

* MAKEDEV script for early era
* For early era - Would you like to explore how the VFS layer handles these device inodes differently from regular files
* devfs + kobject era hotpluggin: /proc/sys/kernel/hotplug multiplexer
* Need to discuss initramfs in context of udev. 
* Fore udev code example, platform_device_id - Explain it in terms of a Device tree, or PCI device, or USB device.
* Device and driver sequencing - What happens if a device is plugged and then kernel module is loaded. Bootup + insmod case.
* How udev works with multiple instances of same device (different minor numbers).

Minimal example of how to create a device in the first era
----------------------------------------------------------

### Step A: The Kernel Driver (my_driver.c)
This minimal driver registers itself under Major number 60 (historically reserved for local/experimental use).
```
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h> /* For older kernels put_user/copy_to_user */

#define MAJOR_NUM 60static char msg[] = "Hello from the static era!\n";static int msg_ptr;
static int device_open(struct inode *inode, struct file *file) {
    msg_ptr = 0;
    MOD_INC_USE_COUNT; /* Old reference counting mechanism */
    return 0;
}
static int device_release(struct inode *inode, struct file *file) {
    MOD_DEC_USE_COUNT;
    return 0;
}
static int device_read(struct file *file, char *buffer, size_t length, loff_t *offset) {
    int bytes_read = 0;
    if (msg[msg_ptr] == 0) return 0;
    while (length && msg[msg_ptr]) {
        put_user(msg[msg_ptr], buffer++);
        length--;
        bytes_read++;
        msg_ptr++;
    }
    return bytes_read;
}
// Map the operationsstatic struct file_operations fops = {
    .read = device_read,
    .open = device_open,
    .release = device_release
};
int init_module(void) {
    // Hard register into the kernel array
    return register_chrdev(MAJOR_NUM, "my_static_dev", &fops);
}
void cleanup_module(void) {
    unregister_chrdev(MAJOR_NUM, "my_static_dev");
}
```
### Step B: Compilation and Loading
The driver is compiled and loaded into the running kernel. At this point, no file appears in /dev.
```
gcc -O2 -DMODULE -D__KERNEL__ -c my_driver.c
insmod my_driver.o
```
### Step C: Manual Userspace Provisioning
The administrator must manually create the file system node matching the driver's parameters.
```
# Syntax: mknod <path> <type: c=char> <major> <minor>
sudo mknod /dev/hello_dev c 60 0
# Set permissions so regular users can access it
sudo chmod 666 /dev/hello_dev
```
### Step D: Testing the Device
The application interacts with the hardcoded node, triggering the kernel execution.
```
cat /dev/hello_dev# Output: Hello from the static era!
```


Minimal Example: Device Creation (Linux 2.4 Era)
------------------------------------------------

This example demonstrates a driver that tells the kernel to automatically spawn its own file node in /dev upon loading.
### The Kernel Driver (devfs_driver.c)
```
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/devfs_fs_kernel.h> // The definitive devfs header

MODULE_LICENSE("GPL");
#define DEVICE_NAME "my_dynamic_node"static devfs_handle_t devfs_handle;
static int device_open(struct inode *inode, struct file *file) {
    return 0;
}
static int device_release(struct inode *inode, struct file *file) {
    return 0;
}
// Minimal file operations mappingstatic struct file_operations fops = {
    .open    = device_open,
    .release = device_release,
};
int init_module(void) {
    // 1. Register with the core VFS layer using an anonymous major number (0)
    int major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) return major;

    // 2. Instruct devfs to instantly create the file node in RAM under /dev/
    devfs_handle = devfs_register(
        NULL,                  // Put directly in base /dev/ (no subdirectory)
        DEVICE_NAME,           // File name: "my_dynamic_node"
        DEVFS_FL_DEFAULT,      // Default flags
        major,                 // The Major number assigned dynamically above
        0,                     // Minor number 0
        S_IFCHR | S_IRUGO,     // Permissions: Character device, Read-only
        &fops,                 // Associated file operations
        NULL                   // Extra driver info pointer
    );

    printk(KERN_INFO "devfs device created dynamically!\n");
    return 0;
}
void cleanup_module(void) {
    // Wipe the node from existence in RAM instantly
    devfs_unregister(devfs_handle);
    unregister_chrdev(0, DEVICE_NAME);
}
```

### Compilation, Loading, and Verification
When an administrator built and loaded this module, the file node materialized completely on its own:
```
# 1. Load the module
insmod devfs_driver.o
# 2. Check the directory - the node appeared without running mknod!
ls -l /dev/my_dynamic_node# Output: crw-r--r--   1 root     root      254,   0 May 17 13:26 /dev/my_dynamic_node
# 3. Remove the module
rmmod devfs_driver
# 4. Check the directory again - it evaporated entirely
ls -l /dev/my_dynamic_node
# Output: ls: /dev/my_dynamic_node: No such file or directory
```