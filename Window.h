#pragma once

#include <QDialog>

class QStandardItem;
class QListView;
class QStandardItemModel;
class QSortFilterProxyModel;
class QLineEdit;

class Window : public QDialog
{
    Q_OBJECT

public:
    Window();

private slots:
    void onAddAccepted();
    void save() const;
    void onItemChanged(QStandardItem *item);

private:
    void load();
    void addItem(const QString &text, const bool checked);

    QListView *m_listView;
    QStandardItemModel *m_listModel;
    QSortFilterProxyModel *m_filterModel;
    QLineEdit *m_addEdit;
};

