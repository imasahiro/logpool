#include "viewer.hpp"

int OGraphicsRectItem::type()
{
	return ClassInfo::OGraphicsRectItem;
}

void OGraphicsRectItem::setJoinedNode(OAbstractNode *joined)
{
	joined_node = joined;
}

OAbstractNode *OGraphicsRectItem::getJoinedNode()
{
	return joined_node;
}

OGraphicsRectItem::OGraphicsRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
	: QGraphicsRectItem(x, y, width, height, parent)
{
	DBG_p("OGraphicsRectItem construct\n");
	mouse_pos = new QPointF();

	QPen p;
	p.setColor(QColor(NODE_FRAME_COLOR));
	p.setCapStyle(Qt::RoundCap);
	p.setWidth(3);
	setPen(p);
	QBrush b(QColor(NODE_BACKGROUND_COLOR));
	setBrush(b);
}

OGraphicsRectItem::~OGraphicsRectItem()
{
	DBG_p("OGraphicsRectItem destruct\n");
	delete(mouse_pos);
}

void OGraphicsRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	DBG_p("OGraphicsRectItem::mousePressEvent\n");
	QPointF tp = this->scenePos();
	QPointF ep = event->scenePos();
	mouse_pos->setX(tp.x() - ep.x());
	mouse_pos->setY(tp.y() - ep.y());
}

void OGraphicsRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	DBG_p("OGraphicsRectItem::mouseReleaseEvent\n");
	mouse_pos->setX(0); // clean mouse_pos
	mouse_pos->setY(0); // clean mouse_pos
}

void OGraphicsRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	DBG_p("OGraphicsRectItem::mouseMoveEvent\n");
	QPointF pf = event->scenePos();
	pf += (*mouse_pos);
	joined_node->setPos(pf);
	OConfNode *cn;
	if ((cn =(dynamic_cast<OConfNode *>(joined_node))) != NULL) {
		cn->moveLine();
	}
}
