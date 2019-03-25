# Interface #

## Data block ##

A central data structure of the gabac interface is the DataBlock. A DataBlock manages a chunk of memory (currently utilizing a std::vector internally) while being aware of the word size of the data inside. The advantage over a traditional std::vector is, that you can adapt the word size dynamically in runtime without having to waste memory. A std::vector<uint64\_t> can only contain 64 bit values, a std::vector<uint32\_t> only 32 bit values. Notice that these are also different types, so a function having std::vector<uint64\_t> as parameters can not be called with std::vector<uint32\_t>. A DataBlock avoids all these problems by encapsulating the memory and retrieving a symbol of the right word size on the fly when accessing it. The DataBlock main constructor takes a size in elements and the requested word size, the remaining interface is very similar to the std::vector. There are also constructors available to create a data block directly from a vector or string.

## Block stepper ##

## Exceptions ##

## Configurations ##

## streams ##

