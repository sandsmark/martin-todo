#include "Window.h"

#include <QCoreApplication>
#include <QSystemTrayIcon>
#include <QFile>
#include <QDir>
#include <QDebug>

#include <QStandardItemModel>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QAction>

#include <QVBoxLayout>

Window::Window(QWidget *parent)
    : QDialog(parent)
{
    setLayout(new QVBoxLayout);

    setWindowFlag(Qt::Dialog);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QPixmap(":/icon.png"));
    m_trayIcon->show();

    QAction *quitAction = new QAction("Quit");
    quitAction->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    addAction(quitAction);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [=]() { setVisible(!isVisible()); });

    QLineEdit *filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Search");
    layout()->addWidget(filterEdit);

    m_listModel = new QStandardItemModel(this);
    m_filterModel = new QSortFilterProxyModel(this);
    m_filterModel->setSourceModel(m_listModel);
    m_listView = new QListView(this);
    layout()->addWidget(m_listView);
    m_listView->setModel(m_filterModel);
    m_listView->setDragDropMode(QAbstractItemView::InternalMove);

    m_addEdit = new QLineEdit(this);
    m_addEdit->setPlaceholderText("Enter todo item...");
    layout()->addWidget(m_addEdit);

    m_addEdit->setFocus();
    m_listView->setFocusProxy(m_addEdit);

    connect(m_addEdit, &QLineEdit::returnPressed, this, &Window::onAddAccepted);
    connect(filterEdit, &QLineEdit::textChanged, m_filterModel, &QSortFilterProxyModel::setFilterFixedString);

    load();

    // We can't connect directly to the itemChanged, because then we manage to delete the item before the signal reaches the qsortfilterproxymodel
    connect(m_filterModel, &QSortFilterProxyModel::dataChanged, this, &Window::resort);
}

Window::~Window()
{
}

void Window::onAddAccepted()
{
    if (m_addEdit->text().isEmpty()) {
        return;
    }

    addItem(m_addEdit->text(), false);
    m_addEdit->clear();
    save();
}

void Window::load()
{
    QFile inputFile(QDir::home().filePath("todo.txt"));
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open" << inputFile.fileName() << inputFile.errorString();
        return;
    }

    QStringList checked;
    QStringList unchecked;

    while (!inputFile.atEnd()) {
        QString inputLine = QString::fromUtf8(inputFile.readLine()).trimmed();
        if (inputLine.isEmpty()) {
            continue;
        }
        const QChar state = inputLine[0];
        inputLine.remove(0, 1);
        if (state == 'x') {
            checked.prepend(inputLine.trimmed());
        } else {
            unchecked.prepend(inputLine.trimmed());
        }
    }

    for (const QString &text : checked) {
        addItem(text, true);
    }

    for (const QString &text : unchecked) {
        addItem(text, false);
    }
}

void Window::save() const
{
    QFile outputFile(QDir::home().filePath("todo.txt"));
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open" << outputFile.fileName() << outputFile.errorString();
        return;
    }
    for (int row=0; row<m_listModel->rowCount(); row++) {
        QStandardItem *item = m_listModel->item(row);
        if (item->text().isEmpty()) {
            continue;
        }
        if (item->checkState() == Qt::Checked) {
            outputFile.write(" x ");
        } else {
            outputFile.write(" - ");
        }

        outputFile.write(item->text().toUtf8());

        outputFile.write("\n");
    }
}

void Window::resort()
{
    QStringList checked, unchecked;
    for (int row=0; row<m_listModel->rowCount(); row++) {
        QStandardItem *item = m_listModel->item(row);
        if (item->checkState() == Qt::Checked) {
            checked.prepend(item->text());
        } else {
            unchecked.prepend(item->text());
        }
    }

    m_listModel->clear();

    for (const QString &text : checked) {
        addItem(text, true);
    }
    for (const QString &text : unchecked) {
        addItem(text, false);
    }

    save();
    m_filterModel->invalidate();
}

void Window::addItem(const QString &text, const bool checked)
{
    if (text.isEmpty()) {
        return;
    }

    QStandardItem *item = new QStandardItem(text);
    item->setCheckable(true);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    item->setDropEnabled(false);
    m_listModel->insertRow(0, item);
}

