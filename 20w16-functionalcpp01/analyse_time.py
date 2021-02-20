#!/usr/bin/env python3
# coding: utf-8

import argparse
import pandas as pd
import numpy as np
from plotnine import *




def process_files(csv_files):
    pdcsvs = [pd.read_csv(csv) for csv in csv_files]
    for i in range(len(pdcsvs)):
        df = pdcsvs[i]
        df['total'] = df['user'] + df['sys']
        df['parallel'] = df['total'] / df['real']
        head, sep, tail = csv_files[i].partition('.')
        df['name'] = head
    return pd.concat(pdcsvs)

if __name__ == '__main__':
    # argument parser
    parser = argparse.ArgumentParser(description='Process some csv files.')
    parser.add_argument('csvs', metavar='FILE', type=str, nargs='+',
                        help='csv files to process')
    parser.add_argument('--logx', action='store_true', help='x-axis is logarithmic')
    parser.add_argument('--logy', action='store_true', help='y-axis is logarithmic')
    parser.add_argument('--name', required=True, help='name of the plots')
    # main arguments
    args = parser.parse_args()
    csvs = args.csvs
    is_logx = args.logx
    is_logy = args.logy
    name = str(args.name)
    # csv to data
    data = process_files(csvs)
    # plot time
    time_plot = (ggplot(data, aes(x='param', y='total', color='factor(name)'))
        + geom_point()
        + geom_line()
        + labs(x='Parameter (value)', y='Time (seconds)', color='')
        )
    if is_logx:
        time_plot += scale_x_log10()
    if is_logy:
        time_plot += scale_y_log10()
    # plot memory
    mem_plot = (ggplot(data, aes(x='param', y='maxmem', color='factor(name)'))
        + geom_point()
        + geom_line()
        + labs(x='Parameter (value)', y='Memory (KiB)', color='')
        )
    if is_logx:
        mem_plot += scale_x_log10()
    if is_logy:
        mem_plot += scale_y_log10()
    # save plots
    time_plot.save(name + '_time.png')
    mem_plot.save(name + '_mem.png')


