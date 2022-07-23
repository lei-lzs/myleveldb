[leveldb之Version相关数据结构_禾夕的博客-CSDN博客](https://blog.csdn.net/u012658346/article/details/45720443)

LevelDb本质上是一系列SSTable文件按一定结构组织起来的文件结构。

单个SSTable文件按照一定的格式存储Data，和数据元信息，读取的时候可以子解析。

Db中的多个Table文件按照LSM(Log-Structured-Merge-Tree)的分层结构存储。我们只需要记录这写文件的元信息，就能知道Db的所有信息。

Version版本：Db数据是动态变化的[Compact]，不同线程可能在不同的时刻访问了数据库，所以需要记录版本信息，版本记录了每一个level的文件元信息，文件编号，Key的范围等。

![img](https://img-blog.csdn.net/20150514155705427?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvdTAxMjY1ODM0Ng==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)