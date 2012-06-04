#ifndef VIEWER_HPP
#define VIEWER_HPP 1

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <QtGui>
#include "util.hpp"
#include <assert.h>

#define SCENE_WIDTH            1024
#define SCENE_HEIGHT           768
#define ELLIPSE_SIZE           10

#define ELLIPSE_SELECTED_COLOR "#ff6633"
#define ELLIPSE_DEFAULT_COLOR  Qt::yellow

#define NODE_FRAME_COLOR       "#003333"
//#define NODE_FRAME_COLOR       Qt::white
#define NODE_BACKGROUND_COLOR  "#cc9999"

using namespace std; //FIX ME!!

class OButton;
class OAbstractNode;
class OConfNode;
class OLogpoolNode;
class OGraphicsEllipseItem;
class OGraphicsRectItem;
class OGraphicsScene;
class OGraphicsTextItem;
class OTextEdit;

namespace ClassInfo {
	typedef enum {
		NoID = QGraphicsItem::UserType,
//		OButton,
		OConfNode,
		OLogpoolNode,
		OGraphicsEllipseItem,
		OGraphicsRectItem,
		OGraphicsScene,
	} ID;
	typedef enum {
		NoName,
		Parent,
		Child1,
		Child2,
		Child3,
		Child4,
		Child5,
		Child6
	} Name;
};

extern OTextEdit *g_t;

class OTextEdit : public QTextEdit
{
	Q_OBJECT;
public:
	OTextEdit();
	~OTextEdit();
	OGraphicsTextItem *sync;
public slots:
	void textSync();
};

//class OInfo {
//private:
//	ClassInfo::ID cid;
//	ClassInfo::Name name;
//public:
//	OInfo(ClassInfo::ID cid);
//	ClassInfo::ID getCid();
//	ClassInfo::Type getType();
//	void setType(ClassInfo::Type _type);
//};

// class OButton : public QPushButton
// {
// };

class OGraphicsTextItem : public QGraphicsTextItem
{
public:
	OGraphicsTextItem(bool d_flag = false);
	~OGraphicsTextItem();
	QString *data;
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	bool sceneEvent(QEvent *event);
};

class OGraphicsEllipseItem : public QGraphicsEllipseItem
{
private:
	OAbstractNode *joined_node;
	QPointF *mouse_pos;
	QPointF *relative_pos;
	OGraphicsEllipseItem *connected;
	QGraphicsLineItem *line; // the line to connected node
	ClassInfo::Name name;

public:
	OGraphicsEllipseItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	~OGraphicsEllipseItem();
	int type();
	void setJoinedNode(OAbstractNode *joined);
	OAbstractNode *getJoinedNode();
	void select();
	void unselect();
	QPointF getRelativePos();
	QPointF getAbsolutePos();
	OGraphicsEllipseItem *getConnected();
	void setConnected(OGraphicsEllipseItem *_connected);
	void setLine(QGraphicsLineItem *_line);
	QGraphicsLineItem *getLine();
	ClassInfo::Name getName();
	void setName(ClassInfo::Name _name); //REMOVE ME!!

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	bool sceneEvent(QEvent *event);
};

class OGraphicsRectItem : public QGraphicsRectItem
{
private:
	OAbstractNode *joined_node;
	QPointF *mouse_pos;

public:
	OGraphicsRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	~OGraphicsRectItem();
	int type();
	void setJoinedNode(OAbstractNode *joined);
	OAbstractNode *getJoinedNode();

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	bool sceneEvent(QEvent *event);
};

class OAbstractNode : public QGraphicsItemGroup
{
public:
	OAbstractNode(QGraphicsItem *parent);
	virtual ~OAbstractNode();
	OGraphicsEllipseItem *getSelectedPoint();
	void setSelectedPoint(OGraphicsEllipseItem *_selected_point);
	static bool connect(OAbstractNode *n1, OGraphicsEllipseItem *e1, OAbstractNode *n2, OGraphicsEllipseItem *e2);
	std::vector<OAbstractNode *> *connected_nodes;

protected:
//pointer to keep selection
	OGraphicsEllipseItem *selected_point;
};

class OLogpoolNode : public OAbstractNode
{
protected:
//items in node
	QGraphicsTextItem *title;
	QGraphicsTextItem *t_child1;
	QGraphicsTextItem *t_child2;
	QGraphicsTextItem *t_child3;
	QGraphicsTextItem *t_child4;
	QGraphicsTextItem *t_child5;
	QGraphicsTextItem *t_child6;
	OGraphicsRectItem *main_rect;
	OGraphicsEllipseItem *e_child1;
	OGraphicsEllipseItem *e_child2;
	OGraphicsEllipseItem *e_child3;
	OGraphicsEllipseItem *e_child4;
	OGraphicsEllipseItem *e_child5;
	OGraphicsEllipseItem *e_child6;
//pointers to point next/previous node
// 	OAbstractNode *n_child1;
// 	OAbstractNode *n_child2;
// 	OAbstractNode *n_child3;
// 	OAbstractNode *n_child4;
// 	OAbstractNode *n_child5;
// 	OAbstractNode *n_child6;

public:
// 	class iterator {
// 	private:
// 		OLogpoolNode *node;
// 	public:
// 		iterator(OLogpoolNode *_node);
// 		~iterator();
// //		iterator toChild1();
// //		iterator toChild2();
// 		void toChild1();
// 		void toChild2();
// 		bool isEnd();
// 		OLogpoolNode *operator*();
// 	};

	OLogpoolNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	virtual ~OLogpoolNode();
	int type() const;
// 	OAbstractNode *getN_child1();
// 	void setN_child1(OAbstractNode *_n_child1);
// 	OAbstractNode *getN_child2();
// 	void setN_child2(OAbstractNode *_n_child2);
// 	OAbstractNode *getN_parent();
//	void setN_parent(OAbstractNode *_n_parent);
	void moveLine();
};

class OConfNode : public OAbstractNode
{
//protected:
public:
//items in node
	QGraphicsTextItem *title;
	QGraphicsTextItem *t_child1;
	QGraphicsTextItem *t_child2;
	OGraphicsRectItem *main_rect;
	OGraphicsEllipseItem *e_child1;
	OGraphicsEllipseItem *e_child2;
	OGraphicsEllipseItem *e_parent;
//pointers to point next/previous node
// 	OAbstractNode *n_child1;
// 	OAbstractNode *n_child2;
// 	OAbstractNode *n_parent;

public:
	class iterator {
	private:
		OConfNode *node;
	public:
		iterator(OConfNode *_node);
		~iterator();
//		iterator toChild1();
//		iterator toChild2();
		void toChild1();
		void toChild2();
		bool isEnd();
		iterator *copy(); //need delete
		OConfNode *operator*();
	};

	typedef enum Status {
		NOP,
		Moving,
		Lining,
		HavePointSelection
	} Status;

	OConfNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	virtual ~OConfNode();
	int type() const;
// 	OAbstractNode *getN_child1();
// 	void setN_child1(OAbstractNode *_n_child1);
// 	OAbstractNode *getN_child2();
// 	void setN_child2(OAbstractNode *_n_child2);
// 	OAbstractNode *getN_parent();
// 	void setN_parent(OAbstractNode *_n_parent);
	OGraphicsEllipseItem *NtoE(int i);
	void moveLine();
	Status status; //REMOVE ME!!

// protected:
// 	void mousePressEvent(QGraphicsSceneMouseEvent *event);
// 	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
// 	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

class OCopyNode : public OConfNode
{
public:
	OCopyNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	~OCopyNode();
	int type() const;
};

class OKeyFilterNode : public OConfNode
{
public:
	OKeyFilterNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	~OKeyFilterNode();
	int type() const;
	OGraphicsTextItem *key;
};

class OValFilterNode : public OConfNode
{
public:
	OValFilterNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	~OValFilterNode();
	int type() const;
	OGraphicsTextItem *key;
	OGraphicsTextItem *val;
	OGraphicsTextItem *fcmp;
};

class OReactNode : public OConfNode
{
public:
	OReactNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	~OReactNode();
	int type() const;
	OGraphicsTextItem *traceID;
	OGraphicsTextItem *key;
};

class OTimeWindowNode : public OConfNode
{
public:
	OTimeWindowNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	~OTimeWindowNode();
	int type() const;
	OGraphicsTextItem *timer;
};

class OStaticNode : public OConfNode
{
public:
	OStaticNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	~OStaticNode();
	int type() const;
	OGraphicsTextItem *timer;
	OGraphicsTextItem *finit;
	OGraphicsTextItem *function;
	OGraphicsTextItem *fexit;
};

class OResponseNode : public OConfNode
{
public:
	OResponseNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = NULL);
	~OResponseNode();
	int type() const;
	OGraphicsTextItem *server;
};

class OGraphicsScene : public QGraphicsScene
{
	Q_OBJECT;
private:
	std::vector<OConfNode*> *nodes;
	std::vector<OConfNode*> *points;
	OLogpoolNode *logpool;

public:
	OGraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = NULL);
	~OGraphicsScene();
	int type();
//	bool onValidPoints(QPointF *pos);
	std::vector<OConfNode*> *getNodes();
	std::string nodeToCode(OConfNode::iterator *n, int *index);
	std::string init;
	std::string chain;
	std::string conf;

public slots:
//	void addNode();
//	void enableMoving();
//	void enableLining();
//	void drawLine();
	void codegen();
};

class Viewer : public QWidget
{
	Q_OBJECT;
private:
	QPushButton *add_button;
//	QPushButton *move_button;
//	QPushButton *line_button;
	QPushButton *codegen_button;
	QComboBox *node_select_box;
	OTextEdit *editor;
	OGraphicsScene *s;
	QGraphicsView *v;
	QVBoxLayout *main_vbl;
	QHBoxLayout *sub_hbl1;
	QHBoxLayout *sub_hbl2;
public:
	Viewer();
	~Viewer();
	void confBox();
public slots:
	void addNode();
};

#endif //VIEWER_HPP
