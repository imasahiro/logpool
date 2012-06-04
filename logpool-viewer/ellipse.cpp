#include "viewer.hpp"

int OGraphicsEllipseItem::type()
{
	return ClassInfo::OGraphicsEllipseItem;
}

ClassInfo::Name OGraphicsEllipseItem::getName()
{
	return name;
}

void OGraphicsEllipseItem::setName(ClassInfo::Name _name)
{
	name = _name;
}

OGraphicsEllipseItem::OGraphicsEllipseItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: QGraphicsEllipseItem(x, y, width, height, parent)
{
	DBG_p("OGraphicsEllipseItem construct\n");
	setFlag(QGraphicsItem::ItemIsSelectable);
	relative_pos = new QPointF(x, y);
	connected = NULL;
	line = NULL;

	QPen p;
	p.setColor(QColor(NODE_FRAME_COLOR));
	p.setCapStyle(Qt::RoundCap);
	p.setWidth(2);
	setPen(p);
	QColor c(ELLIPSE_DEFAULT_COLOR);
	QBrush b(c);
	setBrush(b);
}

OGraphicsEllipseItem::~OGraphicsEllipseItem()
{
	DBG_p("OGraphicsEllipseItem destruct\n");
	delete(relative_pos);
}

OGraphicsEllipseItem *OGraphicsEllipseItem::getConnected()
{
	return connected;
}
void OGraphicsEllipseItem::setConnected(OGraphicsEllipseItem *_connected)
{
	connected = _connected;
}

void OGraphicsEllipseItem::select()
{
	QColor c(ELLIPSE_SELECTED_COLOR);
	QBrush b(c);
	setBrush(b);
	joined_node->setSelectedPoint(this);
}

void OGraphicsEllipseItem::unselect()
{
	QColor c(ELLIPSE_DEFAULT_COLOR);
	QBrush b(c);
	setBrush(b);
	joined_node->setSelectedPoint(NULL);
}

void OGraphicsEllipseItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	DBG_p("OGraphicsEllipseItem::mousePressEvent\n");
	OGraphicsEllipseItem *current_selected_point = joined_node->getSelectedPoint();
	if (current_selected_point == this) {
	}
	else {
		if (current_selected_point != NULL) current_selected_point->unselect();
		joined_node->setSelected(true);
		select();
	}
}

static void extractItemsByType(QList<QGraphicsItem *> *il, int type) //extract items having type
{
	for (int i = 0; i < il->size(); i++) {
		DBG_p("extractItemByType: type = %d\n", il->at(i)->type());
		if (il->at(i)->type() != type) {
			il->removeAt(i);
			i--;
		}
	}
}

static void extractItemsByTypes(QList<QGraphicsItem *> *il, int type1, int type2) //extract items having types
{
	int type;
	for (int i = 0; i < il->size(); i++) {
		type = il->at(i)->type();
		DBG_p("extractItemByType: type = %d\n", type);
		if (type != type1 &&
			type != type2) {
			il->removeAt(i);
			i--;
		}
	}
}

void OGraphicsEllipseItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	DBG_p("OGraphicsEllipseItem::mouseReleaseEvent\n");
	OGraphicsScene *s = dynamic_cast<OGraphicsScene *>(scene());
	OASSET(s != NULL);
	QList<QGraphicsItem *> il = s->selectedItems();
//	extractItemsByType(&il, ClassInfo::OConfNode);
	extractItemsByTypes(&il, ClassInfo::OConfNode, ClassInfo::OLogpoolNode);
	DBG_p("il.size()=%d\n", il.size());
	if (il.size() == 2) {
		OAbstractNode *no1 = dynamic_cast<OAbstractNode*>(il[0]);
		OAbstractNode *no2 = dynamic_cast<OAbstractNode*>(il[1]);
		if (no1 == NULL || no2 == NULL) return;
		OGraphicsEllipseItem *el1 = no1->getSelectedPoint();
		OGraphicsEllipseItem *el2 = no2->getSelectedPoint();
		if (OAbstractNode::connect(no1, el1, no2, el2)) {
			QPointF e_size = QPointF(ELLIPSE_SIZE / 2, ELLIPSE_SIZE / 2);
			QLineF lf(el1->getAbsolutePos() + e_size, el2->getAbsolutePos() + e_size);
			QGraphicsLineItem *gli = new QGraphicsLineItem(lf);
//			gli->setFlag(QGraphicsItem::ItemIsSelectable);
			el1->setLine(gli);
			el2->setLine(gli);
			QPen p;
			p.setColor(ELLIPSE_DEFAULT_COLOR);
			p.setCapStyle(Qt::RoundCap);
			p.setWidth(5);
			gli->setPen(p);
			s->addItem(gli);
		}
		el1->unselect();
		el2->unselect();
		s->clearSelection();
	}
}

void OGraphicsEllipseItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	DBG_p("OGraphicsEllipseItem::mouseMoveEvent\n");
}

// void OGraphicsEllipseItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
// {
// 	DBG_p("OGraphicsEllipseItem::mousePressEvent\n");
// 	QPointF tsp = calcGroupPos(joined_node, this->scenePos());
// 	QPointF esp = calcGroupPos(joined_node, event->scenePos());
// 	cout << tsp.x() << tsp.y() << endl;
// 	cout << esp.x() << esp.y() << endl;
// 	cout << joined_node->scenePos().x() << joined_node->scenePos().y() << endl;

// 	line = new QGraphicsLineItem(QLineF(esp, esp));
// //	line->setFlag(QGraphicsItem::ItemIsSelectable);
// 	QPen p(QColor(Qt::black));
// //	p.setWidth(5);
// 	line->setPen(p);
// 	scene()->addItem(line);
// 	mouse_pos = new QPointF(esp);
// }


// void OGraphicsEllipseItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
// {
// 	DBG_p("OGraphicsEllipseItem::mouseReleaseEvent\n");
// 	OGraphicsScene *current_scene = (OGraphicsScene *)scene();
// 	if (current_scene->onValidPoints(&(event->scenePos()))) {
// 	} else {
// 		current_scene->removeItem(line);
// 		delete(line);
// 	}
// 	mouse_pos = NULL;
// 	line = NULL;
// }

// void OGraphicsEllipseItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
// {
// 	DBG_p("OGraphicsEllipseItem::mouseMoveEvent\n");
// 	std::cout << mouse_pos->x() << ", " << mouse_pos->y() << std::endl;
// 	line->setLine(QLineF(*mouse_pos, event->scenePos()));
// }

void OGraphicsEllipseItem::setJoinedNode(OAbstractNode *joined)
{
	joined_node = joined;
}

OAbstractNode *OGraphicsEllipseItem::getJoinedNode()
{
	return joined_node;
}

QPointF OGraphicsEllipseItem::getRelativePos()
{
	return *relative_pos;
}

QPointF OGraphicsEllipseItem::getAbsolutePos()
{
	return *relative_pos + joined_node->scenePos();
}

void OGraphicsEllipseItem::setLine(QGraphicsLineItem *_line)
{
	line = _line;
}

QGraphicsLineItem *OGraphicsEllipseItem::getLine()
{
	return line;
}
