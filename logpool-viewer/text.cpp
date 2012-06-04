#include "viewer.hpp"

OGraphicsTextItem::OGraphicsTextItem(bool d_flag)
	: QGraphicsTextItem()
{
	if (d_flag) {
		data = new QString();
	}
	else {
		data = NULL;
	}
}

OGraphicsTextItem::~OGraphicsTextItem()
{
}

void OGraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	DBG_p("OGraphicsTextItem::mousePressEvent\n");
	g_t->sync = this;
	if (data == NULL) {
		g_t->setPlainText(toPlainText());
	}
	else {
		g_t->setPlainText(*(data));
	}
}

void OTextEdit::textSync()
{
	if (sync != NULL) {
		if (sync->data == NULL) {
			sync->setPlainText(toPlainText());
		}
		else {
			*(sync->data) = toPlainText();
		}
	}
}

OTextEdit::OTextEdit()
	: QTextEdit()
{
	sync = NULL;
}

OTextEdit::~OTextEdit()
{
}
