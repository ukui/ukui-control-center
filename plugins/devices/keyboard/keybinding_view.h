#ifndef KEYBINDING_VIEW_H
#define KEYBINDING_VIEW_H


#include <QTreeView>
#include <QMainWindow>
#include <QModelIndex>
#include "treemodel.h"

class KeybindingView
{


public:
    KeybindingView(QTreeView *view);
    ~KeybindingView();
    TreeModel *model;
    QTreeView *view;

    void insertChild();
    void insertRow(QStringList list);
    void removeRow();
    void updatemodeltree();

private Q_SLOTS:

};

#endif // KEYBINDING_VIEW_H
