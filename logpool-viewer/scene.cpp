#include "viewer.hpp"

int OGraphicsScene::type()
{
	return ClassInfo::OGraphicsScene;
}

OGraphicsScene::OGraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent)
	: QGraphicsScene(x, y, width, height, parent)
{
	DBG_p("OGraphicsScene construct\n");
	nodes = new std::vector<OConfNode*>();

	logpool = new OLogpoolNode(0, 0, 150, 150);
	addItem(logpool);

	QBrush b(QColor("#cccccc"));
	setBackgroundBrush(b);
}

OGraphicsScene::~OGraphicsScene()
{
	DBG_p("OGraphicsScene destruct\n");
	std::vector<OConfNode*>::size_type vsize = nodes->size();
	for (std::vector<OConfNode*>::size_type i = 0; i < vsize; i++) {
		delete(nodes->at(i));
	}
}

// bool OGraphicsScene::onValidPoints(QPointF *pos)
// {
// 	bool ret = false;
// 	QGraphicsItem *item;
// 	if ((item = itemAt(*pos)) != NULL) {
// 		ret = (item->type() == ClassInfo::OGraphicsEllipseItem ? true : false);
// 	}
// 	return ret;
// }

// void OGraphicsScene::addNode()
// {
// 	DBG_p("OGraphicsScene::addNode\n");
// //	OConfNode *i = new OConfNode(0, 0, 200, 100);
// 	OConfNode *i = static_cast<OConfNode*>(new OKeyFilterNode(0, 0, 200, 100));
// 	addItem(i);
// 	nodes->push_back(i);
// }

// void OGraphicsScene::drawLine()
// {
// 	DBG_p("OGraphicsScene::drawLine\n");
// 	QList<QGraphicsItem *> il = selectedItems();
// 	if (il.size() == 2) {
// 		OConfNode *no1 = dynamic_cast<OConfNode*>(il[0]);
// 		OConfNode *no2 = dynamic_cast<OConfNode*>(il[1]);
// 		assert(no1 != NULL && no2 != NULL);
// 		OGraphicsEllipseItem *el1 = no1->getSelectedPoint();
// 		OGraphicsEllipseItem *el2 = no2->getSelectedPoint();
// 		if (OConfNode::connect(no1, el1, no2, el2)) {
// 			QPointF e_size = QPointF(ELLIPSE_SIZE / 2, ELLIPSE_SIZE / 2);
// 			QLineF lf(el1->getAbsolutePos() + e_size, el2->getAbsolutePos() + e_size);
// 			QGraphicsLineItem *gli = new QGraphicsLineItem(lf);
// 			QPen p;
// 			p.setColor(ELLIPSE_DEFAULT_COLOR);
// 			p.setCapStyle(Qt::RoundCap);
// 			p.setWidth(5);
// 			gli->setPen(p);
// 			addItem(gli);
// 		}
// 		el1->unselect();
// 		el2->unselect();
// 	}
// 	else {
// //		DBG_p("selectedItems.size=%d\n", il.size());
// 		QList<QGraphicsItem*>::iterator itr = il.begin();
// 		QList<QGraphicsItem*>::iterator end = il.end();
// 		while (itr != end) {
// 			OConfNode* n = dynamic_cast<OConfNode*>(*itr);
// 			n->getSelectedPoint()->unselect();
// 			itr++;
// 		}
// 	}
// 	clearSelection();
// }

// void OGraphicsScene::enableMoving()
// {
// 	std::vector<OConfNode*>::size_type vsize = nodes->size();
// 	for (std::vector<OConfNode*>::size_type i = 0; i < vsize; i++) {
// 		(nodes->at(i))->status = OConfNode::Moving;
// 	}
// }

// void OGraphicsScene::enableLining()
// {
// 	std::vector<OConfNode*>::size_type vsize = nodes->size();
// 	for (std::vector<OConfNode*>::size_type i = 0; i < vsize; i++) {
// 		(nodes->at(i))->status = OConfNode::Lining;
// 	}
// }
std::vector<OConfNode*> *OGraphicsScene::getNodes()
{
	return nodes;
}

std::string intToString(int num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}

#define RM_LABEL(qstr) ((qstr).replace(QRegExp(".*:"), ""))

static std::string makeConf(int base, OConfNode *n, std::string type)
{
	std::string ret;
	if (type == "Copy") {
		return ret;
	}

	ret += "\t{\n";
	std::string p_name = "p" + intToString(base);
	if (type == "KeyFilter") {
		OKeyFilterNode *kfn = dynamic_cast<OKeyFilterNode*>(n);
		assert(kfn != NULL);
		ret += "\t\t" + p_name + "->key = \"" + RM_LABEL(kfn->key->toPlainText()).toStdString() + "\";\n";
	}
	else if (type == "ValFilter") {
		OValFilterNode *vfn = dynamic_cast<OValFilterNode*>(n);
		assert(vfn != NULL);
		ret += "\t\t" + p_name + "->key = \"" + RM_LABEL(vfn->key->toPlainText()).toStdString() + "\";\n";
		ret += "\t\t" + p_name + "->val = \"" + RM_LABEL(vfn->val->toPlainText()).toStdString() + "\";\n";
		ret += "\t\t" + p_name + "->fcmp = \"\n" + vfn->fcmp->data->toStdString() + "\n\";\n";
	}
	else if (type == "React") {
		OReactNode *rn = dynamic_cast<OReactNode*>(n);
		assert(rn != NULL);
		ret += "\t\t" + p_name + "->traceID = \"" + RM_LABEL(rn->traceID->toPlainText()).toStdString() + "\";\n";
		ret += "\t\t" + p_name + "->key = \"" + RM_LABEL(rn->key->toPlainText()).toStdString() + "\";\n";
	}
	else if (type == "TimeWindow") {
		OTimeWindowNode *twn = dynamic_cast<OTimeWindowNode*>(n);
		assert(twn != NULL);
		ret += "\t\t" + p_name + "->timer = " + RM_LABEL(twn->timer->toPlainText()).toStdString() + ";\n";
	}
	else if (type == "Static") {
		OStaticNode *sn = dynamic_cast<OStaticNode*>(n);
		assert(sn != NULL);
		ret += "\t\t" + p_name + "->timer = " + RM_LABEL(sn->timer->toPlainText()).toStdString() + ";\n";
		ret += "\t\t" + p_name + "->finit = \"\n" + sn->finit->data->toStdString() + "\n\";\n";
		ret += "\t\t" + p_name + "->function = \"\n" + sn->function->data->toStdString() + "\n\";\n";
		ret += "\t\t" + p_name + "->fexit = \"\n" + sn->fexit->data->toStdString() + "\n\";\n";
	}
	else if (type == "Response") {
		OResponseNode *rn = dynamic_cast<OResponseNode*>(n);
		assert(rn != NULL);
		ret += "\t\t" + p_name + "->server = \"" + RM_LABEL(rn->server->toPlainText()).toStdString() + "\";\n";
	}
	ret += "\t};\n";
	return ret;
}

std::string OGraphicsScene::nodeToCode(OConfNode::iterator *n, int *index)
{
	std::string ret;
	int base = *index;
	std::string type = (**n)->title->toPlainText().toStdString();
	init += "\tstruct pool_plugin_" + type + " *p" + intToString(*index) + " = POOL_PLUGIN_CLONE(pool_plugin_" + type + ");\n";
	conf += makeConf(base, **n, type);
	OConfNode::iterator *copy_n = n->copy();
	n->toChild1();
	copy_n->toChild2();
	if (!(n->isEnd())) {
		chain += "\tp" + intToString(base) + "->base.apply = &p" + intToString(++(*index)) + "->base;\n";
		nodeToCode(n, index);
	}
	if (!(copy_n->isEnd())) {
		chain += "\tp" + intToString(base) + "->base.failed = &p" + intToString(++(*index)) + "->base;\n";
		nodeToCode(copy_n, index);
	}

	return ret;
}

void OGraphicsScene::codegen()
{
	DBG_p("OGraphicsScene::codegen()\n");
	std::ofstream ofs("mod_logpool.c");
	ofs << "#include \"plugins/pool/pool_plugin.h\"\n";
	ofs << "struct pool_plugin *cpu_usage_init(struct bufferevent *bev)\n";
	ofs << "{\n";

	std::vector<OAbstractNode*> *cn = logpool->connected_nodes;
	int size = cn->size();
	int idx = 0;
	for (int i = 0; i < size; i++) {
		OConfNode::iterator *itr = new OConfNode::iterator(dynamic_cast<OConfNode*>(cn->at(i)));
		if (!(itr->isEnd())) {
			DBG_p("call nodeToCode()\n");
			nodeToCode(itr, &idx);
			idx++;
			ofs << init << chain << conf << std::endl;
			init.erase(0);
			chain.erase(0);
			conf.erase(0);
		}
	}
	ofs << "\treturn pool_plugin_init((struct pool_plugin *)p0);\n";
	ofs << "}\n";
	ofs << std::endl;
}
