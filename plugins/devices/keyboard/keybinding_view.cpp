#include "keybinding_view.h"
#include <QFile>

KeybindingView::KeybindingView(QTreeView *view):view(view){

    QStringList headers;
    headers << ("Title") << ("Description");

    QFile file("/home/kylin/default.txt");
    file.open(QIODevice::ReadOnly);
    model = new TreeModel();
    view->setModel(model);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setFocusPolicy(Qt::NoFocus);


    updatemodeltree();
    for (int column = 0; column < model->columnCount(); ++column)
        view->resizeColumnToContents(column);

}

void KeybindingView::updatemodeltree()
{
    QStringList desktop_shortcuts,system_shortcuts;
    desktop_shortcuts<<"desktop shortcuts"<<"";
    system_shortcuts<<"system shortcuts"<<"";
    insertRow(desktop_shortcuts);
    insertRow(system_shortcuts);


}
void KeybindingView::insertChild()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    for (int column = 0; column < model->columnCount(index); ++column) {
        QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant("[No data]"), Qt::EditRole);
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }

    view->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);

}

void KeybindingView::insertRow(QStringList list)
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
        QModelIndex child = model->index(index.row()+1, column, index.parent());
        model->setData(child, QVariant(list[column]), Qt::EditRole);
    }
}

void KeybindingView::removeRow()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();
    if (model->removeRow(index.row(), index.parent()));
}

