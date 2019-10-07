#include "Window.h"

#include <QCoreApplication>
#include <QSystemTrayIcon>
#include <QFile>
#include <QDir>
#include <QDebug>

#include <QStandardItemModel>
#include <QListView>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QAction>

#include <QVBoxLayout>

Window::Window()
{
    setWindowFlag(Qt::Dialog);
    setLayout(new QVBoxLayout);

    // Tray icon
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(QPixmap(":/icon.png"), this);
    trayIcon->show();
    connect(trayIcon, &QSystemTrayIcon::activated, this, [=]() { setVisible(!isVisible()); });

    // Quit action (ctrl+q I guess)
    QAction *quitAction = new QAction("Quit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    addAction(quitAction);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    // Model with the actual tasks
    m_listModel = new QStandardItemModel(this);

    // For sorting checked tasks below unchecked, and searching/filtering
    m_filterModel = new QSortFilterProxyModel(this);
    m_filterModel->setRecursiveFilteringEnabled(true);
    m_filterModel->setSourceModel(m_listModel);
    m_filterModel->setSortRole(Qt::CheckStateRole);

    // List of tasks
    m_listView = new QListView(this);
    m_listView->setModel(m_filterModel);
    m_listView->setDragDropMode(QAbstractItemView::InternalMove);

    // Add new item edit
    m_addEdit = new QLineEdit(this);
    m_addEdit->setPlaceholderText("Enter todo item...");
    m_addEdit->setClearButtonEnabled(true);
    connect(m_addEdit, &QLineEdit::returnPressed, this, &Window::onAddAccepted);

    // Search edit
    QLineEdit *filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Search...");
    filterEdit->setClearButtonEnabled(true);
    connect(filterEdit, &QLineEdit::textChanged, m_filterModel, &QSortFilterProxyModel::setFilterFixedString);
    connect(filterEdit, SIGNAL(returnPressed()), m_listView, SLOT(setFocus())); // the qOverride crap is so fucking ugly

    // Filter/search on top, the list in the middle, entering new item at the bottom
    layout()->addWidget(filterEdit);
    layout()->addWidget(m_listView);
    layout()->addWidget(m_addEdit);

    m_addEdit->setFocus();

    load();

    connect(m_filterModel, &QSortFilterProxyModel::rowsMoved, this, &Window::save);
    connect(m_listModel, &QStandardItemModel::itemChanged, this, &Window::onItemChanged);

    resize(500, 750);
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

        outputFile.write(item->text().toUtf8() + '\n');
    }
}

void Window::onItemChanged(QStandardItem *item)
{
    if (item->text().isEmpty()) {
        m_listModel->removeRow(item->row(), item->parent() ? item->parent()->index() : QModelIndex());
    }
    m_filterModel->sort(0);

    save();
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

