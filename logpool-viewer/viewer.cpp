#include "viewer.hpp"

OTextEdit *g_t;

void Viewer::confBox()
{
#define _COPY 0
	node_select_box->addItem(QString("Copy"));
#define _KEYFILTER 1
	node_select_box->addItem(QString("KeyFilter"));
#define _VALFILTER 2
	node_select_box->addItem(QString("ValFilter"));
#define _REACT 3
	node_select_box->addItem(QString("react"));
#define _TIMEWINDOW 4
	node_select_box->addItem(QString("timewindow"));
#define _STATICS 5
	node_select_box->addItem(QString("statics"));
#define _RESPONSE 6
	node_select_box->addItem(QString("response"));
}

void Viewer::addNode()
{
	DBG_p("OGraphicsView::addNode\n");
//	OConfNode *i = new OConfNode(0, 0, 200, 100);
	OConfNode *i;
	switch (node_select_box->currentIndex()) {
	case _COPY:
		i = static_cast<OConfNode*>(new OCopyNode(0, 0, 200, 100));
		break;
	case _KEYFILTER:
		i = static_cast<OConfNode*>(new OKeyFilterNode(0, 0, 200, 100));
		break;
	case _VALFILTER:
		i = static_cast<OConfNode*>(new OValFilterNode(0, 0, 200, 100));
		break;
	case _REACT:
		i = static_cast<OConfNode*>(new OReactNode(0, 0, 200, 100));
		break;
	case _TIMEWINDOW:
		i = static_cast<OConfNode*>(new OTimeWindowNode(0, 0, 200, 100));
		break;
	case _STATICS:
		i = static_cast<OConfNode*>(new OStaticNode(0, 0, 200, 100));
		break;
	case _RESPONSE:
		i = static_cast<OConfNode*>(new OResponseNode(0, 0, 200, 100));
		break;
	default:
		OABORT("Viewer::addNode, currentIndex of node_select_box is invalie\n");
	}
	s->addItem(i);
	s->getNodes()->push_back(i);
}

Viewer::Viewer()
{
	add_button = new QPushButton(QString("add"));
//	move_button = new QPushButton(QString("move"));
//	line_button = new QPushButton(QString("line"));
	codegen_button = new QPushButton(QString("codegen"));
	node_select_box = new QComboBox();
	confBox();
	editor = new OTextEdit();
	g_t = editor;

	s = new OGraphicsScene(0, 0, SCENE_WIDTH, SCENE_HEIGHT);
	v = new QGraphicsView(s);

	sub_hbl1 = new QHBoxLayout();
	sub_hbl1->addWidget(node_select_box);
	sub_hbl1->addWidget(add_button);
//	hbl->addWidget(move_button);
//	hbl->addWidget(line_button);
	sub_hbl1->addWidget(codegen_button);
	sub_hbl1->addStretch();

	sub_hbl2 = new QHBoxLayout();
	sub_hbl2->addWidget(v);
	sub_hbl2->addWidget(editor);

	main_vbl = new QVBoxLayout();
	main_vbl->addLayout(sub_hbl1);
	main_vbl->addLayout(sub_hbl2);

	QObject::connect(add_button, SIGNAL(clicked()), this, SLOT(addNode()));
//	QObject::connect(move_button, SIGNAL(clicked()), s, SLOT(enableMoving()));
//	QObject::connect(line_button, SIGNAL(clicked()), s, SLOT(drawLine()));
	QObject::connect(codegen_button, SIGNAL(clicked()), s, SLOT(codegen()));
	QObject::connect(editor, SIGNAL(textChanged()), editor, SLOT(textSync()));

	setLayout(main_vbl);
};

Viewer::~Viewer()
{
	delete(add_button);
	delete(codegen_button);
	delete(node_select_box);
	delete(s);
	delete(v);
	delete(sub_hbl1);
	delete(sub_hbl2);
	delete(main_vbl);
}
