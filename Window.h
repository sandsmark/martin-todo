#ifndef WINDOW_H
#define WINDOW_H

#include <QDialog>

class QSystemTrayIcon;
class QListView;
class QStandardItemModel;
class QSortFilterProxyModel;
class QLineEdit;

class Window : public QDialog
{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);
    ~Window();

private slots:
    void onAddAccepted();
    void save();

private:
    void load();
    void addItem(const QString &text, const bool checked);

    QSystemTrayIcon *m_trayIcon;

    QListView *m_listView;
    QStandardItemModel *m_listModel;
    QSortFilterProxyModel *m_filterModel;

    QLineEdit *m_addEdit;
};
#endif // WINDOW_H
