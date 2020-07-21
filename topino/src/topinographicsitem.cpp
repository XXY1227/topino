#include "include/topinographicsitem.h"

TopinoGraphicsItem::TopinoGraphicsItem(int newitemid, QGraphicsItem* parent) : QGraphicsItem(parent) {
    itemid = newitemid;
}

TopinoGraphicsItem::~TopinoGraphicsItem() {
}

int TopinoGraphicsItem::getItemid() const {
    return itemid;
}

void TopinoGraphicsItem::setItemid(int value) {
    itemid = value;
}
