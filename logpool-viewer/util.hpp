//#ifdef ODEBUG
#define DBG_p(fmt, ...)    fprintf(stderr, fmt, ## __VA_ARGS__)
#define OABORT(fmt, ...)						\
	do {										\
		fprintf(stderr, fmt, ## __VA_ARGS__);	\
		abort();								\
	} while(false)
#define OASSET(cond)       assert(cond)
#define printi(i)          std::cout << (i) << std::endl;
//#else
//#define DBG_p(fmt, ...)
//#endif //ODEBUG

QPointF calcGroupPos(QGraphicsItem *group, QPointF newScenePos);
