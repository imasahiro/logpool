#include "viewer.hpp"

int OConfNode::type() const
{
	return ClassInfo::OConfNode;
}

int OCopyNode::type() const
{
	return ClassInfo::OConfNode;
}

int OKeyFilterNode::type() const
{
	return ClassInfo::OConfNode;
}

int OValFilterNode::type() const
{
	return ClassInfo::OConfNode;
}

int OReactNode::type() const
{
	return ClassInfo::OConfNode;
}

int OTimeWindowNode::type() const
{
	return ClassInfo::OConfNode;
}

int OStaticNode::type() const
{
	return ClassInfo::OConfNode;
}

int OResponseNode::type() const
{
	return ClassInfo::OConfNode;
}

OCopyNode::OCopyNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: OConfNode(x, y, width, height, parent)
{
	title->setPlainText(QString("Copy"));
	t_child1->setPlainText(QString("apply"));
	t_child2->setPlainText(QString("failed"));

	title->setPos(10, 0);
	QFont f = title->font();
	f.setBold(true);
	f.setPointSize(15);
	title->setFont(f);
	t_child1->setPos(150, 10);
	t_child2->setPos(150, 30);
}

OCopyNode::~OCopyNode()
{
}

OKeyFilterNode::OKeyFilterNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: OConfNode(x, y, width, height, parent)
{
	title->setPlainText(QString("KeyFilter"));
	t_child1->setPlainText(QString("matched"));
	t_child2->setPlainText(QString("failed"));

	title->setPos(10, 0);
	QFont f = title->font();
	f.setBold(true);
	f.setPointSize(15);
	title->setFont(f);
	t_child1->setPos(130, 10);
	t_child2->setPos(150, 30);

//init conf
	key = new OGraphicsTextItem();
	key->setPlainText("key:");
	key->setPos(0, 30);
	addToGroup(key);
}

OKeyFilterNode::~OKeyFilterNode()
{
	delete(key);
}

OValFilterNode::OValFilterNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: OConfNode(x, y, width, height, parent)
{
	title->setPlainText(QString("ValFilter"));
	t_child1->setPlainText(QString("matched"));
	t_child2->setPlainText(QString("failed"));

	title->setPos(10, 0);
	QFont f = title->font();
	f.setBold(true);
	f.setPointSize(15);
	title->setFont(f);
	t_child1->setPos(130, 10);
	t_child2->setPos(150, 30);

//init conf
	key = new OGraphicsTextItem();
	key->setPlainText("key:");
	addToGroup(key);
	key->setPos(0, 30);
	val = new OGraphicsTextItem();
	val->setPlainText("value:");
	addToGroup(val);
	val->setPos(0, 50);
	fcmp = new OGraphicsTextItem(true);
	fcmp->setPlainText("fcmp(script)");
	*(fcmp->data) = QString("fcmp(script)");
	addToGroup(fcmp);
	fcmp->setPos(0, 70);
}

OValFilterNode::~OValFilterNode()
{
}

OReactNode::OReactNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: OConfNode(x, y, width, height, parent)
{
	title->setPlainText(QString("React"));
	t_child1->setPlainText(QString("changeed"));
	t_child2->setPlainText(QString("nochanged"));

	title->setPos(10, 0);
	QFont f = title->font();
	f.setBold(true);
	f.setPointSize(15);
	title->setFont(f);
	t_child1->setPos(125, 10);
	t_child2->setPos(115, 30);

//init conf
	traceID = new OGraphicsTextItem();
	traceID->setPlainText("traceID:");
	addToGroup(traceID);
	traceID->setPos(0, 30);
	key = new OGraphicsTextItem();
	key->setPlainText("key:");
	addToGroup(key);
	key->setPos(0, 50);
}

OReactNode::~OReactNode()
{
}

OTimeWindowNode::OTimeWindowNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: OConfNode(x, y, width, height, parent)
{
	title->setPlainText(QString("TimeWindow"));
	t_child1->setPlainText(QString("buffered"));
	t_child2->setPlainText(QString("expired"));

	title->setPos(10, 0);
	QFont f = title->font();
	f.setBold(true);
	f.setPointSize(15);
	title->setFont(f);
	t_child1->setPos(130, 10);
	t_child2->setPos(140, 30);

//init conf
	timer = new OGraphicsTextItem();
	timer->setPlainText("timer:");
	addToGroup(timer);
	timer->setPos(0, 30);
}

OTimeWindowNode::~OTimeWindowNode()
{
}

OStaticNode::OStaticNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: OConfNode(x, y, width, height, parent)
{
	title->setPlainText(QString("Static"));
	t_child1->setPlainText(QString("next"));
	t_child2->setPlainText(QString("failed"));

	title->setPos(10, 0);
	QFont f = title->font();
	f.setBold(true);
	f.setPointSize(15);
	title->setFont(f);
	t_child1->setPos(155, 10);
	t_child2->setPos(150, 30);

//init conf
	timer = new OGraphicsTextItem();
	timer->setPlainText("timer:");
	addToGroup(timer);
	timer->setPos(0, 20);
	finit = new OGraphicsTextItem(true);
	finit->setPlainText("finit(script)");
	*(finit->data) = QString("finit(script)");
	addToGroup(finit);
	finit->setPos(0, 40);
	function = new OGraphicsTextItem(true);
	function->setPlainText("function(script)");
	*(function->data) = QString("function(script)");
	addToGroup(function);
	function->setPos(0, 60);
	fexit = new OGraphicsTextItem(true);
	fexit->setPlainText("fexit(script)");
	*(fexit->data) = QString("fexit(script)");
	addToGroup(fexit);
	fexit->setPos(0, 80);
}

OStaticNode::~OStaticNode()
{
}

OResponseNode::OResponseNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: OConfNode(x, y, width, height, parent)
{
	title->setPlainText(QString("Response"));
	t_child1->setPlainText(QString("apply"));
	t_child2->setPlainText(QString("failed"));

	title->setPos(10, 0);
	QFont f = title->font();
	f.setBold(true);
	f.setPointSize(15);
	title->setFont(f);
	t_child1->setPos(150, 10);
	t_child2->setPos(150, 30);

//init conf
	server = new OGraphicsTextItem();
	server->setPlainText("server:");
	addToGroup(server);
	server->setPos(0, 20);
}

OResponseNode::~OResponseNode()
{
}

OAbstractNode::OAbstractNode(QGraphicsItem *parent)
	: QGraphicsItemGroup(parent)
{
	connected_nodes = new std::vector<OAbstractNode*>(ClassInfo::Child6, NULL); //TODO
}

OAbstractNode::~OAbstractNode()
{
}

OConfNode::OConfNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: OAbstractNode(parent)
{
	if (parent == NULL) parent = this;
	status = NOP;
	selected_point = NULL;
	setFlags(QGraphicsItem::ItemIsSelectable);
	main_rect = new OGraphicsRectItem(x, y, width, height, parent);
	title = new QGraphicsTextItem(parent);
	t_child1 = new QGraphicsTextItem(parent);
	t_child2 = new QGraphicsTextItem(parent);
//	e_child1 = new OGraphicsEllipseItem(x + width - ELLIPSE_SIZE, y, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_child1 = new OGraphicsEllipseItem(x + width - ELLIPSE_SIZE, y + 20, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_child1->setName(ClassInfo::Child1);
//	e_child2 = new OGraphicsEllipseItem(x + width - ELLIPSE_SIZE, y + ELLIPSE_SIZE, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_child2 = new OGraphicsEllipseItem(x + width - ELLIPSE_SIZE, y + 40, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_child2->setName(ClassInfo::Child2);
	e_parent = new OGraphicsEllipseItem(x, y, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_parent->setName(ClassInfo::Parent);
	main_rect->setJoinedNode(this);
//title->setJoinedNode(this) //TODO?
	e_child1->setJoinedNode(this);
	e_child2->setJoinedNode(this);
	e_parent->setJoinedNode(this);
	addToGroup(main_rect);
	addToGroup(title);
	addToGroup(e_child1);
	addToGroup(e_child2);
	addToGroup(e_parent);
}

OConfNode::~OConfNode()
{
	delete(main_rect);
	delete(e_child1);
	delete(e_child2);
	delete(e_parent);
}

// void OConfNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
// {
// 	DBG_p("OConfNode::mousePressEvent\n");
// 	switch (status) {
// 	case Moving:
// 		main_rect->mousePressEvent(event);
// 		break;
// 	case Lining:
// 		e_child1->mousePressEvent(event);
// 		break;
// 	default:
// 		DBG_p("OConfNode::mousePressEvent error!!");
// 	}
// }

// void OConfNode::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
// {
// 	DBG_p("OConfNode::mouseMoveEvent\n");
// 	switch (status) {
// 	case Moving:
// 		main_rect->mouseMoveEvent(event);
// 		break;
// 	case Lining:
// 		e_child1->mouseMoveEvent(event);
// 		break;
// 	default:
// 		DBG_p("OConfNode::mouseMoveEvent error!!");
// 	}
// }
// void OConfNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
// {
// 	DBG_p("OConfNode::mouseReleaseEvent\n");
// 	switch (status) {
// 	case Moving:
// 		main_rect->mouseReleaseEvent(event);
// 		break;
// 	case Lining:
// 		e_child1->mouseReleaseEvent(event);
// 		break;
// 	default:
// 		DBG_p("OConfNode::mouseReleaseEvent error!!");
// 	}
// }

// OAbstractNode *OConfNode::getN_child1()
// {
// 	return n_child1;
// }

// void OConfNode::setN_child1(OAbstractNode *_n_child1)
// {
// 	n_child1 = _n_child1;
// }

// OAbstractNode *OConfNode::getN_child2()
// {
// 	return n_child2;
// }

// void OConfNode::setN_child2(OAbstractNode *_n_child2)
// {
// 	n_child2 = _n_child2;
// }

// OAbstractNode *OConfNode::getN_parent()
// {
// 	return n_parent;
// }

// void OConfNode::setN_parent(OAbstractNode *_n_parent)
// {
// 	n_parent = _n_parent;
// }

OConfNode::iterator::iterator(OConfNode *_node)
{
	node = _node;
}

OConfNode::iterator::~iterator()
{
}

OConfNode::iterator *OConfNode::iterator::copy()
{
	OConfNode::iterator *ret = new OConfNode::iterator(node);
	return ret;
}

void OConfNode::iterator::toChild1()
{
	if (node != NULL) {
		node = dynamic_cast<OConfNode*>(node->connected_nodes->at(ClassInfo::Child1));
//		assert(node != NULL);
	}
//	return *this;
}

void OConfNode::iterator::toChild2()
{
	if (node != NULL) {
		node = dynamic_cast<OConfNode*>(node->connected_nodes->at(ClassInfo::Child2));
//		assert(node != NULL);
	}
//	return *this;
}

bool OConfNode::iterator::isEnd()
{
	if (node == NULL) {
		return true;
	}
	else {
		return false;
	}
}

OConfNode *OConfNode::iterator::operator*()
{
	return node;
}

bool OAbstractNode::connect(OAbstractNode *no1, OGraphicsEllipseItem *el1, OAbstractNode *no2, OGraphicsEllipseItem *el2)
{
	ClassInfo::Name na1 = el1->getName();
	ClassInfo::Name na2 = el2->getName();
	assert(na1 != ClassInfo::NoName && na2 != ClassInfo::NoName);
	bool ret;
	(*(no1->connected_nodes))[na1] = no2;
	(*(no2->connected_nodes))[na2] = no1;
	el1->setConnected(el2);
	el2->setConnected(el1);
	ret = true;
	return ret;
}

OGraphicsEllipseItem *OAbstractNode::getSelectedPoint()
{
	return selected_point;
}

void OAbstractNode::setSelectedPoint(OGraphicsEllipseItem *_selected_point)
{
	selected_point = _selected_point;
}


OGraphicsEllipseItem *OConfNode::NtoE(int i)
{
	OGraphicsEllipseItem *ret;
	switch(i) {
	case ClassInfo::NoName:
		break;
	case ClassInfo::Parent:
		ret = e_parent;
		break;
	case ClassInfo::Child1:
		ret = e_child1;
		break;
	case ClassInfo::Child2:
		ret = e_child2;
		break;
// 	case ClassInfo::Child3:
// 		ret = e_parent;
// 		break;
// 	case ClassInfo::Child4:
// 		ret = e_parent;
// 		break;
// 	case ClassInfo::Child5:
// 		ret = e_parent;
// 		break;
// 	case ClassInfo::Child6:
// 		ret = e_parent;
// 		break;
	}
	return ret;
}

#define ONODE_MOVELINE(e_elem)											                       \
	OGraphicsEllipseItem *e_elem_connected = e_elem->getConnected();	                       \
	QGraphicsLineItem *line = e_elem->getLine();						                       \
	QLineF lf(e_elem->getAbsolutePos() + e_size, e_elem_connected->getAbsolutePos() + e_size); \
	line->setLine(lf);

void OConfNode::moveLine()
{
	DBG_p("OGraphicsRectItem::moveLine\n");
	QPointF e_size = QPointF(ELLIPSE_SIZE / 2, ELLIPSE_SIZE / 2);
	int size = connected_nodes->size();
	for (int i = 0; i < size; i++) {
		OAbstractNode *n = connected_nodes->at(i);
		if (n != NULL) {
			OGraphicsEllipseItem *e_elem = NtoE(i);
			OGraphicsEllipseItem *e_elem_connected = e_elem->getConnected();
			QGraphicsLineItem *line = e_elem->getLine();
			QLineF lf(e_elem->getAbsolutePos() + e_size, e_elem_connected->getAbsolutePos() + e_size);
			line->setLine(lf);
		}
	}
// 	if (n_parent != NULL) {
// 		ONODE_MOVELINE(e_parent);
// 	}
// 	if (n_child1 != NULL) {
// 		ONODE_MOVELINE(e_child1);
// 	}
// 	if (n_child2 != NULL) {
// 		ONODE_MOVELINE(e_child2);
// 	}
}

int OLogpoolNode::type() const
{
	return ClassInfo::OLogpoolNode;
}

OLogpoolNode::OLogpoolNode(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: OAbstractNode(parent)
{
	if (parent == NULL) parent = this;
	selected_point = NULL;
	setFlags(QGraphicsItem::ItemIsSelectable);
	main_rect = new OGraphicsRectItem(x, y, width, height, parent);
	title = new QGraphicsTextItem(parent);
	title->setPlainText("Logpool");
	QFont f = title->font();
	f.setBold(true);
	f.setPointSize(15);
	title->setFont(f);
	t_child1 = new QGraphicsTextItem(parent);
	t_child2 = new QGraphicsTextItem(parent);
	t_child3 = new QGraphicsTextItem(parent);
	t_child4 = new QGraphicsTextItem(parent);
	t_child5 = new QGraphicsTextItem(parent);
	t_child6 = new QGraphicsTextItem(parent);
	e_child1 = new OGraphicsEllipseItem(x + width - ELLIPSE_SIZE, y + 30, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_child1->setName(ClassInfo::Child1);
	e_child2 = new OGraphicsEllipseItem(x + width - ELLIPSE_SIZE, y + 60, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_child2->setName(ClassInfo::Child2);
	e_child3 = new OGraphicsEllipseItem(x + width - ELLIPSE_SIZE, y + 90, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_child3->setName(ClassInfo::Child3);
	e_child4 = new OGraphicsEllipseItem(x + 30, y + height - ELLIPSE_SIZE, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_child4->setName(ClassInfo::Child4);
	e_child5 = new OGraphicsEllipseItem(x + 60, y + height - ELLIPSE_SIZE, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_child5->setName(ClassInfo::Child5);
	e_child6 = new OGraphicsEllipseItem(x + 90, y + height - ELLIPSE_SIZE, ELLIPSE_SIZE, ELLIPSE_SIZE, parent);
	e_child6->setName(ClassInfo::Child6);
	main_rect->setJoinedNode(this);
//title->setJoinedNode(this) //TODO?
	e_child1->setJoinedNode(this);
	e_child2->setJoinedNode(this);
	e_child3->setJoinedNode(this);
	e_child4->setJoinedNode(this);
	e_child5->setJoinedNode(this);
	e_child6->setJoinedNode(this);
	addToGroup(main_rect);
	addToGroup(title);
	addToGroup(e_child1);
	addToGroup(e_child2);
	addToGroup(e_child3);
	addToGroup(e_child4);
	addToGroup(e_child5);
	addToGroup(e_child6);
}

OLogpoolNode::~OLogpoolNode()
{
// 	delete(main_rect);
// 	delete(e_child1);
// 	delete(e_child2);
// 	delete(e_parent);
}
