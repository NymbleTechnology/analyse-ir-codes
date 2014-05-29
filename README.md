analyse-ir-codes
================

This is a post-processing tool I wrote as a side-job (and was never paid for - thanks, guys :-/ ).

The company in question had purchased a data set of raw IR codes whose accuracy was pretty poor.
To the point that some codes didn't work reliably, or at all. This code runs through that data,
analysing the codes to identify the coding scheme they were based upon, and then using that
knowledge to tweak the data for greater accuracy.

Engineers often confuse the relative simplicity of IR code encoding for ease of implementing
IR capture and/or transmission that works reliably for real products. You have been warned :)

- Paul
(a.k.a. 'A Helpful Person' on the Pronto forums)
