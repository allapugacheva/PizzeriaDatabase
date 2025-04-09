#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setPort(5432);
    db.setDatabaseName("Pizza");
    db.setUserName("postgres");
    db.setPassword("1234");

    if (!db.open())
        qCritical() << "Не удалось подключиться к базе данных: " << db.lastError().text();

    QSqlQuery query;
    query.exec("SELECT name, query FROM query");

    while (query.next()) {
        queryList.append(query.value(1).toString());
        queryNameList.append(query.value(0).toString());
    }

    QStringListModel *model = new QStringListModel(this);
    model->setStringList(queryNameList);

    ui->queryList->setModel(model);

    ui->queryResultView->setAlternatingRowColors(true);

    connect(ui->executeExistButton, &QPushButton::clicked, this, &MainWindow::onExecuteButtonCliked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteButtonClicked);
    connect(ui->executeNewButton, &QPushButton::clicked, this, &MainWindow::onExecuteNewButtonClicked);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::onAddButtonClicked);
    connect(ui->saveToFileButton, &QPushButton::clicked, this, &MainWindow::onSaveButtonClicked);
}

MainWindow::~MainWindow() {

    delete ui;
}

void MainWindow::executeQuery(QString query)
{
    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery(query, db);

    QSqlQuery executedQuery = model->query();
    if (executedQuery.lastError().isValid())
        QMessageBox::critical(nullptr, "Ошибка SQL-запроса", executedQuery.lastError().text());
    else {
        QSqlRecord record = model->record();
        for (int i = 0; i < record.count(); ++i) {
            QString fieldName = record.fieldName(i);
            model->setHeaderData(i, Qt::Horizontal, QObject::tr(fieldName.toUtf8()));
        }

        ui->queryResultView->setModel(model);
        ui->queryResultView->resizeColumnsToContents();
    }
}

void MainWindow::saveToFile(QAbstractItemModel *model, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    int columns = model->columnCount();
    int rows = model->rowCount();

    QVector<int> columnWidths(columns, 0);

    for (int col = 0; col < columns; ++col) {
        columnWidths[col] = model->headerData(col, Qt::Horizontal).toString().length();
    }

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            QString text = model->index(row, col).data().toString();
            columnWidths[col] = qMax(columnWidths[col], text.length());
        }
    }

    for (int col = 0; col < columns; ++col) {
        QString header = model->headerData(col, Qt::Horizontal).toString();
        out << header.leftJustified(columnWidths[col] + 2);  // +2 — отступ
    }
    out << "\n";

    for (int col = 0; col < columns; ++col) {
        out << QString("-").repeated(columnWidths[col]) + "  ";
    }
    out << "\n";

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            QString text = model->index(row, col).data().toString();
            out << text.leftJustified(columnWidths[col] + 2);
        }
        out << "\n";
    }

    file.close();
}

void MainWindow::onExecuteButtonCliked()
{
    QModelIndex current = ui->queryList->currentIndex();
    if (current.isValid())
        executeQuery(queryList[current.row()]);
}

void MainWindow::onDeleteButtonClicked()
{
    QModelIndex current = ui->queryList->currentIndex();
    if (current.isValid()) {
        QString queryString = "DELETE FROM query WHERE name = '" + queryNameList[current.row()] + "'";
        QSqlQuery query;
        query.exec(queryString);

        queryList.removeAt(current.row());
        queryNameList.removeAt(current.row());

        QStringListModel *model = qobject_cast<QStringListModel*>(ui->queryList->model());
        model->removeRow(current.row());
        ui->queryList->update();
    }
}

void MainWindow::onExecuteNewButtonClicked()
{
    QString query = ui->queryEdit->toPlainText();
    if (!query.isEmpty())
        executeQuery(query);
}

void MainWindow::onAddButtonClicked()
{
    QString query = ui->queryEdit->toPlainText();
    if (!query.isEmpty()) {
        bool ok;
        QString text = QInputDialog::getText(this,
                                             "Введите название",
                                             "Название запроса:",
                                             QLineEdit::Normal,
                                             "",
                                             &ok);

        if (ok && !text.isEmpty()) {
            queryList.append(query);
            queryNameList.append(text);

            QStringListModel *model = qobject_cast<QStringListModel*>(ui->queryList->model());
            model->setStringList(queryNameList);
            ui->queryList->update();

            QSqlQuery sqlQuery;
            sqlQuery.prepare("INSERT INTO query (name, query) VALUES (:name, :query)");
            sqlQuery.bindValue(":name", text);
            sqlQuery.bindValue(":query", query);
            sqlQuery.exec();
        }
    }
}

void MainWindow::onSaveButtonClicked()
{
    if (ui->queryResultView->model() != nullptr) {

        bool ok;
        QString text = QInputDialog::getText(this,
                                             "Введите название",
                                             "Название файла:",
                                             QLineEdit::Normal,
                                             "",
                                             &ok);

        if (ok && !text.isEmpty())
            saveToFile(ui->queryResultView->model(), text + ".txt");
    }
}
