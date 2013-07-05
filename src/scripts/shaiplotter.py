#!/usr/bin/env python
import sys
import pickle
import matplotlib.pylab as pl
import numpy as np

if __name__ == '__main__':
    _, resfile = sys.argv
    with open(resfile, 'r') as fp:
        d = pickle.load(fp)
    l = zip(d['configs'], d['results'])
    r_values = [x for x in l if x[0].keys() == ['r']]
    L_values = [x for x in l if x[0].keys() == ['L']]

    r_batch_add_line = [(x[0]['r'], x[1]['add_rate_batch_ms']) for x in r_values]
    r_add_line = [(x[0]['r'], x[1]['add_rate_per_op']) for x in r_values]
    r_batch_mult_line = [(x[0]['r'], x[1]['mult_rate_batch_ms']) for x in r_values]
    r_mult_line = [(x[0]['r'], x[1]['mult_rate_per_op']) for x in r_values]

    L_batch_add_line = [(x[0]['L'], x[1]['add_rate_batch_ms']) for x in L_values]
    L_add_line = [(x[0]['L'], x[1]['add_rate_per_op']) for x in L_values]
    L_batch_mult_line = [(x[0]['L'], x[1]['mult_rate_batch_ms']) for x in L_values]
    L_mult_line = [(x[0]['L'], x[1]['mult_rate_per_op']) for x in L_values]

    def first(x):
        return [e[0] for e in x]
    def second(x):
        return [e[1] for e in x]

    def mkplot(rbatchvalues, rvalues, lbatchvalues, lvalues, tpe, logscale=True):
        fig = pl.figure()
        ax = pl.subplot(111)
        plfn = ax.semilogy if logscale else ax.plot
        plfn(first(rbatchvalues), (second(rbatchvalues)), 'r')
        plfn(first(rvalues), (second(rvalues)), 'r--')
        plfn(first(lbatchvalues), (second(lbatchvalues)), 'b')
        plfn(first(lvalues), (second(lvalues)), 'b--')
        ax.set_xlabel('parameter value')
        ax.set_ylabel('ms per op')
        ax.set_title(tpe)
        ax.legend(('r-batch', 'r-op', 'L-batch', 'L-op'), loc='center right')
        fig.savefig(tpe + '.pdf')

    print r_add_line
    print L_add_line

    mkplot(r_batch_add_line, r_add_line, L_batch_add_line, L_add_line, 'add')
    mkplot(r_batch_mult_line, r_mult_line, L_batch_mult_line, L_mult_line, 'mult')


