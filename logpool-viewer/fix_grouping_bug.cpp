#include "viewer.hpp"

bool OGraphicsEllipseItem::sceneEvent(QEvent *event)
{
	switch (event->type()) {
	case QEvent::GraphicsSceneMousePress:
		mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
		break;
	case QEvent::GraphicsSceneMouseRelease:
		mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
		break;
	case QEvent::GraphicsSceneMouseMove:
		mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
		break;
	default:
		DBG_p("OGraphicsEllipseItem::sceneEvent event ignored\n");
	}
	event->setAccepted(true);
	return true;
}

bool OGraphicsRectItem::sceneEvent(QEvent *event)
{
	switch (event->type()) {
	case QEvent::GraphicsSceneMousePress:
		mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
		break;
	case QEvent::GraphicsSceneMouseRelease:
		mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
		break;
	case QEvent::GraphicsSceneMouseMove:
		mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
		break;
	default:
		DBG_p("OGraphicsRectItem::sceneEvent event ignored\n");
	}
	event->setAccepted(true);
	return true;
}

bool OGraphicsTextItem::sceneEvent(QEvent *event)
{
	switch (event->type()) {
	case QEvent::GraphicsSceneMousePress:
		mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
		break;
	case QEvent::GraphicsSceneMouseRelease:
		mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
		break;
	case QEvent::GraphicsSceneMouseMove:
		mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
		break;
	default:
		DBG_p("OGraphicsTextItem::sceneEvent event ignored\n");
	}
	event->setAccepted(true);
	return true;
}

QPointF calcGroupPos(QGraphicsItem *group, QPointF newScenePos)
{
    QPointF origin = group->sceneBoundingRect().topLeft() - group->scenePos();
    QPointF delta = newScenePos - origin;
    return delta;
}
