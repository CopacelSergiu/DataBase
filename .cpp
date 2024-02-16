#include <QApplication>
#include <QMainWindow>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>
#include <QMap>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>

// Structura pentru o coloană a unei tabele
struct Column {
    QString name;
    QString type;
    QList<QString> values; // Lista de valori pentru coloana respectivă
};

// Structura pentru o tabelă
struct Table {
    QString name;
    QList<Column> columns;
};

// Structura pentru o bază de date
struct Database {
    QString name;
    QList<Table> tables;
};

// Map pentru a stoca bazele de date create
QMap<QString, Database> databases;

// Combobox-ul pentru bazele de date disponibile
QComboBox* databaseComboBox;

// Combobox-ul pentru tabelele existente în grupul "CREATE TABLE"
QComboBox* existingTablesComboBox;

// Combobox-ul pentru tabelele existente în grupul "INSERT INTO"
QComboBox* existingTablesComboBoxInsert;

// Combobox-ul pentru coloanele disponibile în grupul "INSERT INTO"
QComboBox* availableColumnsComboBox;

// Combobox-urile pentru grupul "SELECT"
QComboBox* comboBoxColumnSelect;
QComboBox* comboBoxTableFrom;
QComboBox* comboBoxColumnWhere;
QComboBox* comboBoxCondition;

// Fereastra pentru conținutul bazei de date deschise
QMainWindow* contentWindow = nullptr;

// Variabilă pentru layout-ul grupului de elemente
QGridLayout* groupLayout = nullptr;

// Funcție pentru deschiderea paginii cu conținutul bazei de date selectate
void openDatabaseContentPage(const QString& databaseName) {
    if (databases.contains(databaseName)) {
        if (contentWindow == nullptr) {
            contentWindow = new QMainWindow;
            contentWindow->setWindowTitle("Database Content");
        }

        QTableWidget* tableWidget = new QTableWidget;
        contentWindow->setCentralWidget(tableWidget);

        QStringList headers;
        headers << "Table" << "Column" << "Value";
        tableWidget->setColumnCount(headers.size());
        tableWidget->setHorizontalHeaderLabels(headers);

        QList<Table> tables = databases[databaseName].tables;
        // Iterăm prin fiecare tabel din baza de date
        for (const Table& table : tables) {
            QTableWidgetItem* tableNameItem = new QTableWidgetItem(table.name);

            // Iterăm prin fiecare coloană din tabel
            for (const Column& column : table.columns) {
                QTableWidgetItem* columnNameItem = new QTableWidgetItem(column.name);

                // Iterăm prin fiecare valoare din coloană
                for (const QString& value : column.values) {
                    int row = tableWidget->rowCount(); // Obținem numărul curent de rânduri
                    tableWidget->insertRow(row); // Inserăm un nou rând
                    tableWidget->setItem(row, 0, tableNameItem); // Setăm numele tabelului în prima coloană
                    tableWidget->setItem(row, 1, columnNameItem); // Setăm numele coloanei în a doua coloană
                    tableWidget->setItem(row, 2, new QTableWidgetItem(value)); // Setăm valoarea în a treia coloană
                }
            }
        }


        contentWindow->show();

        // Populăm comboboxurile "SELECT", "FROM" și "WHERE" cu numele tabelelor și al coloanelor
        comboBoxTableFrom->clear();
        comboBoxColumnSelect->clear();
        comboBoxColumnWhere->clear();

        for (const Table& table : tables) {
            comboBoxTableFrom->addItem(table.name);
            for (const Column& column : table.columns) {
                comboBoxColumnSelect->addItem(column.name);
                comboBoxColumnWhere->addItem(column.name);
            }
        }
    }
    else {
        QMessageBox::warning(nullptr, "Error", "Database '" + databaseName + "' does not exist.");
    }
}
// Funcție pentru crearea și salvarea bazei de date
void createDatabase(const QString& databaseName) {
    if (databases.contains(databaseName)) {
        QMessageBox::warning(nullptr, "Error", "Database '" + databaseName + "' already exists.");
    }
    else {
        Database db;
        db.name = databaseName;
        databases.insert(databaseName, db);
        QMessageBox::information(nullptr, "Database Creation", "Database created successfully: " + databaseName);

        databaseComboBox->addItem(databaseName);
    }
}

// Funcție pentru adăugarea unei noi coloane la o tabelă existentă într-o bază de date dată
void addColumnToExistingTable(const QString& databaseName, const QString& tableName, const QString& columnName, const QString& columnType) {
    if (databases.contains(databaseName)) {
        QList<Table>& tables = databases[databaseName].tables;
        for (Table& table : tables) {
            if (table.name == tableName) {
                Column newColumn;
                newColumn.name = columnName;
                newColumn.type = columnType;
                table.columns.append(newColumn);
                QMessageBox::information(nullptr, "Column Added", "Column added successfully to table '" + tableName + "'.");
                openDatabaseContentPage(databaseName);
                return;
            }
        }
        QMessageBox::warning(nullptr, "Error", "Table '" + tableName + "' does not exist in database '" + databaseName + "'.");
    }
    else {
        QMessageBox::warning(nullptr, "Error", "Database '" + databaseName + "' does not exist.");
    }
}

// Funcție pentru crearea și salvarea unei noi tabele într-o bază de date existentă
void createTableInDatabase(const QString& databaseName, const QString& tableName, const QList<QPair<QString, QString>>& columns) {
    if (databases.contains(databaseName)) {
        QList<Table>& tables = databases[databaseName].tables;
        for (const Table& table : tables) {
            if (table.name == tableName) {
                QMessageBox::warning(nullptr, "Error", "Table '" + tableName + "' already exists in database '" + databaseName + "'.");
                return;
            }
        }

        Table newTable;
        newTable.name = tableName;
        for (const QPair<QString, QString>& column : columns) {
            Column newColumn;
            newColumn.name = column.first;
            newColumn.type = column.second;
            newTable.columns.append(newColumn);
        }
        tables.append(newTable);
        QMessageBox::information(nullptr, "Table Creation", "Table created successfully: " + tableName);

        openDatabaseContentPage(databaseName);

        // Update the existing tables combo box
        existingTablesComboBox->addItem(tableName);
        existingTablesComboBoxInsert->addItem(tableName); // Adăugăm numele tabelului în combobox-ul pentru tabele disponibile în grupul "INSERT INTO"
    }
    else {
        QMessageBox::warning(nullptr, "Error", "Database '" + databaseName + "' does not exist.");
    }
}

// Funcție pentru inserarea unei valori într-o coloană a unei tabele
void insertValueIntoColumn(const QString& databaseName, const QString& tableName, const QString& columnName, const QString& value) {
    if (databases.contains(databaseName)) {
        QList<Table>& tables = databases[databaseName].tables;
        for (Table& table : tables) {
            if (table.name == tableName) {
                // Verificăm dacă coloana există în tabela selectată
                for (Column& column : table.columns) {
                    if (column.name == columnName) {
                        // Salvăm valoarea în structura de date
                        // Aici inserăm valoarea în structura de date
                        // Afișăm valoarea în fereastra bazei de date
                        column.values.append(value);
                        openDatabaseContentPage(databaseName);
                        QMessageBox::information(nullptr, "Insert Successful", "Value inserted successfully into column " + columnName + " of table " + tableName + ".");
                        return;
                    }
                }
                QMessageBox::warning(nullptr, "Error", "Column " + columnName + " does not exist in table " + tableName + ".");
                return;
            }
        }
        QMessageBox::warning(nullptr, "Error", "Table " + tableName + " does not exist in the database.");
    }
    else {
        QMessageBox::warning(nullptr, "Error", "Database " + databaseName + " does not exist.");
    }
}
void executeSelectQuery(const QString& selectedColumn, const QString& selectedTable, const QString& selectedWhereColumn, const QString& condition, const QString& value) {
    // Verificați dacă baza de date selectată există în map-ul databases
    QString selectedDatabase = databaseComboBox->currentText();
    if (!databases.contains(selectedDatabase)) {
        QMessageBox::warning(nullptr, "Error", "Selected database does not exist.");
        return;
    }

    // Verificați dacă tabela selectată există în baza de date selectată
    const Database& selectedDB = databases[selectedDatabase];
    bool tableFound = false;
    for (const Table& table : selectedDB.tables) {
        if (table.name == selectedTable) {
            tableFound = true;
            // Verificați dacă coloana pentru condiția WHERE există în tabela selectată
            bool whereColumnFound = false;
            for (const Column& column : table.columns) {
                if (column.name == selectedWhereColumn) {
                    whereColumnFound = true;
                    // Executați operația SELECT
                    QMessageBox::information(nullptr, "SELECT Result", "Performing SELECT query...\nSelected Column: " + selectedColumn + "\nSelected Table: " + selectedTable + "\nWhere Column: " + selectedWhereColumn + "\nCondition: " + condition + "\nValue: " + value);
                    // Aici puteți implementa codul pentru a obține și afișa datele din baza de date
                    return;
                }
            }
            if (!whereColumnFound) {
                QMessageBox::warning(nullptr, "Error", "Selected WHERE column does not exist in the selected table.");
                return;
            }
        }
    }
    if (!tableFound) {
        QMessageBox::warning(nullptr, "Error", "Selected table does not exist in the selected database.");
        return;
    }
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Interfață cu grupuri de elemente");

    QGridLayout* gridLayout = new QGridLayout;

    QStringList comenziSQL = { "CREATE DATABASE", "OPEN DATABASE", "CLOSE DATABASE", "CREATE TABLE", "INSERT INTO", "SELECT", "CREATE INDEX" };
    for (int i = 0; i < comenziSQL.size(); ++i) {
        QGroupBox* groupBox = new QGroupBox(comenziSQL[i]);
        groupLayout = new QGridLayout(groupBox);

        if (comenziSQL[i] == "CREATE DATABASE") {
            QLabel* label = new QLabel("NUME");
            QLineEdit* lineEdit = new QLineEdit;
            QPushButton* button = new QPushButton("CREATE");
            groupLayout->addWidget(label, 0, 0);
            groupLayout->addWidget(lineEdit, 0, 1);
            groupLayout->addWidget(button, 1, 0, 1, 2);

            QObject::connect(button, &QPushButton::clicked, [=]() {
                QString databaseName = lineEdit->text();
                if (!databaseName.isEmpty()) {
                    createDatabase(databaseName);
                }
                else {
                    QMessageBox::warning(nullptr, "Error", "Please enter a database name.");
                }
                });
        }
        else if (comenziSQL[i] == "OPEN DATABASE") {
            databaseComboBox = new QComboBox;
            QPushButton* openButton = new QPushButton("OPEN");
            groupLayout->addWidget(databaseComboBox, 0, 0);
            groupLayout->addWidget(openButton, 0, 1);

            QObject::connect(openButton, &QPushButton::clicked, [=]() {
                QString selectedDatabase = databaseComboBox->currentText();
                openDatabaseContentPage(selectedDatabase);
                });
        }
        else if (comenziSQL[i] == "CLOSE DATABASE") {
            QPushButton* button = new QPushButton("CLOSE DATABASE");
            groupLayout->addWidget(button, 0, 0, 1, 3);

            QObject::connect(button, &QPushButton::clicked, [=]() {
                if (contentWindow)
                    contentWindow->close();
                });
        }
        else if (comenziSQL[i] == "CREATE TABLE") {
            QLineEdit* tableNameLineEdit = new QLineEdit;
            QLabel* columnNameLabel = new QLabel("COLUMN NAME");
            QLineEdit* columnNameLineEdit = new QLineEdit;
            existingTablesComboBox = new QComboBox; // Combobox-ul pentru tabelele existente în grupul "CREATE TABLE"
            QComboBox* dataTypeComboBox = new QComboBox;
            dataTypeComboBox->addItem("INT");
            dataTypeComboBox->addItem("BOOL");
            dataTypeComboBox->addItem("STRING");
            dataTypeComboBox->addItem("FLOAT");
            QPushButton* addColumnButton = new QPushButton("ADD COLUMN");
            QPushButton* createTableButton = new QPushButton("CREATE TABLE");
            groupLayout->addWidget(new QLabel("TABLE NAME"), 0, 0);
            groupLayout->addWidget(tableNameLineEdit, 0, 1);
            groupLayout->addWidget(new QLabel("EXISTING TABLES"), 1, 0);
            groupLayout->addWidget(existingTablesComboBox, 1, 1);
            groupLayout->addWidget(columnNameLabel, 2, 0);
            groupLayout->addWidget(columnNameLineEdit, 2, 1);
            groupLayout->addWidget(new QLabel("DATA TYPE"), 3, 0);
            groupLayout->addWidget(dataTypeComboBox, 3, 1);
            groupLayout->addWidget(addColumnButton, 4, 0, 1, 2);
            groupLayout->addWidget(createTableButton, 5, 0, 1, 2);

            QObject::connect(addColumnButton, &QPushButton::clicked, [=]() {
                QString tableName = existingTablesComboBox->currentText();
                QString columnName = columnNameLineEdit->text();
                QString columnType = dataTypeComboBox->currentText();
                if (!columnName.isEmpty()) {
                    addColumnToExistingTable(databaseComboBox->currentText(), tableName, columnName, columnType);
                }
                else {
                    QMessageBox::warning(nullptr, "Error", "Please enter a column name.");
                }
                });

            QObject::connect(createTableButton, &QPushButton::clicked, [=]() {
                QString databaseName = databaseComboBox->currentText();
                QString tableName = tableNameLineEdit->text();
                if (!databaseName.isEmpty() && !tableName.isEmpty()) {
                    QList<QPair<QString, QString>> columns;
                    QLineEdit* nameLineEdit = columnNameLineEdit;
                    QComboBox* typeComboBox = dataTypeComboBox;
                    QString columnName = nameLineEdit->text();
                    QString columnType = typeComboBox->currentText();
                    if (!columnName.isEmpty())
                        columns.append(qMakePair(columnName, columnType));
                    if (!columns.isEmpty())
                        createTableInDatabase(databaseName, tableName, columns);
                    else
                        QMessageBox::warning(nullptr, "Error", "Please add at least one column.");
                }
                else {
                    QMessageBox::warning(nullptr, "Error", "Please enter table name.");
                }
                });
        }
        else if (comenziSQL[i] == "INSERT INTO") {
            QPushButton* button = new QPushButton("INSERT");
            QLabel* labelTable = new QLabel("AVAILABLE TABLE");
            existingTablesComboBoxInsert = new QComboBox; // Combobox-ul pentru tabelele disponibile în grupul "INSERT INTO"
            QLabel* labelColumn = new QLabel("AVAILABLE COLUMN");
            availableColumnsComboBox = new QComboBox; // Combobox-ul pentru coloanele disponibile în grupul "INSERT INTO"
            QLabel* labelValue = new QLabel("VALUE");
            QLineEdit* lineEditValue = new QLineEdit;
            QLabel* labelColumnContent = new QLabel("COLUMN CONTENT");
            QComboBox* comboBoxColumnContent = new QComboBox;

            groupLayout->addWidget(labelTable, 0, 0);
            groupLayout->addWidget(existingTablesComboBoxInsert, 0, 1);
            groupLayout->addWidget(labelColumn, 1, 0);
            groupLayout->addWidget(availableColumnsComboBox, 1, 1);
            groupLayout->addWidget(labelColumnContent, 2, 0);
            groupLayout->addWidget(comboBoxColumnContent, 2, 1);
            groupLayout->addWidget(labelValue, 3, 0);
            groupLayout->addWidget(lineEditValue, 3, 1);
            groupLayout->addWidget(button, 4, 0, 1, 2);

            QObject::connect(existingTablesComboBoxInsert, QOverload<int>::of(&QComboBox::activated), [=]() {
                QString tableName = existingTablesComboBoxInsert->currentText();
                const Database& database = databases[databaseComboBox->currentText()];
                for (const Table& table : database.tables) {
                    if (table.name == tableName) {
                        availableColumnsComboBox->clear();
                        for (const Column& column : table.columns) {
                            availableColumnsComboBox->addItem(column.name);
                        }
                        break;
                    }
                }
                });

            QObject::connect(availableColumnsComboBox, QOverload<int>::of(&QComboBox::activated), [=]() {
                QString tableName = existingTablesComboBoxInsert->currentText();
                QString columnName = availableColumnsComboBox->currentText();
                comboBoxColumnContent->clear(); // Curățăm combobox-ul "COLUMN CONTENT"
                const Database& database = databases[databaseComboBox->currentText()];
                for (const Table& table : database.tables) {
                    if (table.name == tableName) {
                        for (const Column& column : table.columns) {
                            if (column.name == columnName) {
                                // Populăm combobox-ul "COLUMN CONTENT" cu valorile din coloana selectată
                                for (const QString& value : column.values) {
                                    comboBoxColumnContent->addItem(value);
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
                });


            QObject::connect(button, &QPushButton::clicked, [=]() {
                QString tableName = existingTablesComboBoxInsert->currentText();
                QString columnName = availableColumnsComboBox->currentText();
                QString value = lineEditValue->text();
                QString columnContent = comboBoxColumnContent->currentText();
                if (!value.isEmpty()) {
                    insertValueIntoColumn(databaseComboBox->currentText(), tableName, columnName, value);
                }
                else {
                    QMessageBox::warning(nullptr, "Error", "Please enter a value.");
                }
                });
        }


        else if (comenziSQL[i] == "SELECT") {
            QPushButton* button = new QPushButton("SELECT");
            QLabel* labelSelect = new QLabel("SELECT");
            QLabel* labelFrom = new QLabel("FROM");
            QLabel* labelWhere = new QLabel("WHERE");
            comboBoxColumnSelect = new QComboBox;
            comboBoxColumnSelect->addItem("Column 1");
            comboBoxColumnSelect->addItem("Column 2");
            comboBoxColumnSelect->addItem("Column 3");
            comboBoxTableFrom = new QComboBox;
            comboBoxTableFrom->addItem("Table 1");
            comboBoxTableFrom->addItem("Table 2");
            comboBoxTableFrom->addItem("Table 3");
            comboBoxColumnWhere = new QComboBox;
            comboBoxColumnWhere->addItem("Column 1");
            comboBoxColumnWhere->addItem("Column 2");
            comboBoxColumnWhere->addItem("Column 3");
            comboBoxCondition = new QComboBox;
            comboBoxCondition->addItem("=");
            comboBoxCondition->addItem(">=");
            comboBoxCondition->addItem("<=");
            comboBoxCondition->addItem(">");
            comboBoxCondition->addItem("<");
            comboBoxCondition->addItem("!=");
            QLabel* labelValueWhere = new QLabel("VALUE");
            QLineEdit* lineEditValueWhere = new QLineEdit;
            groupLayout->addWidget(labelSelect, 0, 0);
            groupLayout->addWidget(comboBoxColumnSelect, 0, 1);
            groupLayout->addWidget(labelFrom, 1, 0);
            groupLayout->addWidget(comboBoxTableFrom, 1, 1);
            groupLayout->addWidget(labelWhere, 2, 0);
            groupLayout->addWidget(comboBoxColumnWhere, 2, 1);
            groupLayout->addWidget(comboBoxCondition, 2, 2);
            groupLayout->addWidget(labelValueWhere, 3, 0);
            groupLayout->addWidget(lineEditValueWhere, 3, 1);
            groupLayout->addWidget(button, 4, 0, 1, 3);
            QObject::connect(button, &QPushButton::clicked, [=]() {
                QString selectedColumn = comboBoxColumnSelect->currentText();
                QString selectedTable = comboBoxTableFrom->currentText();
                QString selectedWhereColumn = comboBoxColumnWhere->currentText();
                QString condition = comboBoxCondition->currentText();
                QString value = lineEditValueWhere->text();

                executeSelectQuery(selectedColumn, selectedTable, selectedWhereColumn, condition, value);
                });

        }
        else {
            QPushButton* button = new QPushButton("Button");
            groupLayout->addWidget(button, 0, 0, 1, 3);
        }

        groupBox->setLayout(groupLayout);
        int row = i / 3;
        int col = i % 3;
        gridLayout->addWidget(groupBox, row, col);
    }

    QWidget* centralWidget = new QWidget(&window);
    centralWidget->setLayout(gridLayout);
    window.setCentralWidget(centralWidget);

    window.show();

    return app.exec();
}
