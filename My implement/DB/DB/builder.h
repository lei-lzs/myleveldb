
#ifndef _LEVELDB_BUILDER_HH
#define _LEVELDB_BUILDER_HH

#include "status.h"
#include <string>

class Env;
class Options;
class TableCache;
class Iterator;
class FileMetaData;


Status BuildTable(const std::string& dbname, Env* env, const Options& options,
    TableCache* table_cache, Iterator* iter, FileMetaData* meta);


#endif

