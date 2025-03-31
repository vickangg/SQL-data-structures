// PROJECT IDENTIFIER: C0F4DFE8B340D81183C208F70F9D2D797908754D

#include <algorithm>
#include <deque>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <getopt.h>
#include <sstream>

#include "Field.h"

using namespace std;

// CREATE <tablename> <N> <coltype1> <coltype2> ... <coltypeN> <colname1> <colname2> ... <colnameN>
// N = columns (> 0), each column contains coltype with colname
// valid coltypes are {double, int, bool, string}

// enum class ColumnType { String, Int, Double, Bool };

struct Table
{
    uint16_t N;
    string tablename;
    vector<string> colnames;
    vector<ColumnType> coltypes;
    vector<vector<variant<int, double, bool, string>>> rows;
    uint32_t currentIndex = 0;

    unordered_map<string, vector<uint32_t>> hashIndex;                  // hash index
    map<variant<int, double, bool, string>, vector<uint32_t>> bstIndex; // bst index
    string indexedColumn;                                               // name of column that's indexed
    string indexType;                                                   // "hash" or "bst" or empty
};

struct printTable
{
    vector<ColumnType> printColTypes;
    vector<string> printColNames;
};

class Silly
{
public:
    /*****************VARIABLES********************/
    bool quiet = false;
    string errorJunk;
    unordered_map<string, Table> database;
    string command;

    void printHelp(char *command)
    {
        cout << "Usage: " << command << "  -h | -q\n";
    } // printHelp()

    /*****************GETMODE**********************/

    void getMode(int argc, char *argv[])
    {
        opterr = false; // Let us handle all error output for command line options
        int choice;
        int option_index = 0;
        option long_options[] = {
            {"help", no_argument, nullptr, 'h'},
            {"quiet", no_argument, nullptr, 'q'},
            {nullptr, 0, nullptr, '\0'}};

        while ((choice = getopt_long(argc, argv, "hq", long_options, &option_index)) != -1)
        {
            switch (choice)
            {
            case 'h':
                printHelp(*argv);
                exit(0);

            case 'q':
                quiet = true;
                break;

            default:
                cerr << "Error: Unknown command line option" << endl;
                exit(1);
            } // switch
        } // while
    }

    // CREATE <tablename> <N> <coltype1> <coltype2> ... <coltypeN> <colname1> <colname2> ... <colnameN>
    // N = columns (> 0), each column contains coltype with colname
    // valid colltypes are {double, int, bool, string}
    void create()
    {
        Table creation;
        cin >> creation.tablename >> creation.N;

        creation.colnames.resize(creation.N);
        creation.coltypes.resize(creation.N);

        if (database.find(creation.tablename) == database.end())
        {
            for (uint16_t i = 0; i < creation.N; ++i)
            {
                string coltype;
                cin >> coltype;

                if (coltype == "string")
                {
                    creation.coltypes[i] = ColumnType::String;
                }
                else if (coltype == "int")
                {
                    creation.coltypes[i] = ColumnType::Int;
                }
                else if (coltype == "double")
                {
                    creation.coltypes[i] = ColumnType::Double;
                }
                else
                {
                    creation.coltypes[i] = ColumnType::Bool;
                }
            }

            // Second loop: Read column names and associate with column types
            for (uint16_t i = 0; i < creation.N; ++i)
            {
                string colname;
                cin >> colname; // Read column name

                // Store column name and its corresponding type in the coltypes map
                creation.colnames[i] = colname;
            }

            // Store the table in the database
            database[creation.tablename] = creation;

            cout << "New table " << creation.tablename << " with column(s) ";
            for (const auto &output : creation.colnames)
            {
                cout << output << " ";
            }
            cout << "created\n";
        }

        else
        {
            cout << "Error during CREATE: Cannot create already existing table " << creation.tablename << endl;
            getline(cin, errorJunk);
            return;
        }
    }

    void remove()
    {
        string removeTable;
        cin >> removeTable;

        if (database.find(removeTable) != database.end())
        {
            database.erase(removeTable);
            cout << "Table " << removeTable << " removed\n";
        }
        else
        {
            cout << "Error during REMOVE: " << removeTable << " does not name a table in the database\n";
        }
    }

    void insert()
    {
        string insertTable;
        uint32_t rows;
        string junk; // Used for reading non-value data (e.g., 'insert')

        // Read the first line: tablename and the number of rows to insert
        cin >> junk >> insertTable >> rows >> junk;

        // Check if the table exists in the database
        if (database.find(insertTable) != database.end())
        {
            // Resize the rows vector to accommodate the new rows
            database[insertTable].rows.reserve(database[insertTable].rows.size() + rows);

            // Loop over the number of rows to be inserted
            for (uint32_t i = 0; i < rows; i++)
            {
                vector<variant<int, double, bool, string>> newRow;

                // Loop over each column in the table to insert the correct value type
                for (uint32_t j = 0; j < database[insertTable].colnames.size(); j++)
                {
                    // Variable to hold the value for this column in the current row
                    string value;
                    // Based on the column type, read the appropriate value
                    switch (database[insertTable].coltypes[j])
                    {
                    case ColumnType::String:
                    {
                        cin >> value;
                        newRow.emplace_back(value);
                        break;
                    }
                    case ColumnType::Double:
                    {
                        double doubleVal;
                        cin >> doubleVal;
                        newRow.emplace_back(doubleVal);
                        break;
                    }
                    case ColumnType::Int:
                    {
                        int intVal;
                        cin >> intVal;
                        newRow.emplace_back(intVal);
                        break;
                    }
                    case ColumnType::Bool:
                    {
                        bool boolVal;
                        cin >> boolVal;
                        newRow.emplace_back(boolVal);
                        break;
                    }
                    }
                }

                // After reading all the columns for the current row, add it to the table's rows
                database[insertTable].rows.push_back(newRow);
            }
        }

        else
        {
            cout << "Error during INSERT: " << insertTable << " does not name a table in the database\n";
            // Error during <COMMAND>: <tablename> does not name a table in the database
            getline(cin, errorJunk);
            return;
        }
        cout << "Added " << rows << " rows to " << database[insertTable].tablename << " from position " << database[insertTable].currentIndex << " to ";
        database[insertTable].currentIndex += rows - 1;
        cout << database[insertTable].currentIndex << "\n";
        /// Added <N> rows to <tablename> from position <startN> to <endN>
    }

    // PRINT implementation
    void print()
    {
        string printName;
        uint32_t numCols;
        cin >> command >> printName >> numCols;

        // Check if table exists
        if (database.find(printName) == database.end())
        {
            cout << "Error during PRINT: " << printName << " does not name a table in the database\n";
            getline(cin, command);
            return;
        }

        Table &table = database[printName];
        printTable columnsToPrint;

        // Read and validate columns to print
        for (uint32_t i = 0; i < numCols; i++)
        {
            cin >> command;
            auto it = find(table.colnames.begin(), table.colnames.end(), command);
            if (it == table.colnames.end())
            {
                cout << "Error during PRINT: " << command << " does not name a column in " << printName << "\n";
                getline(cin, command);
                return;
            }
            size_t colIndex = static_cast<size_t>(distance(table.colnames.begin(), it));
            columnsToPrint.printColNames.push_back(command);
            columnsToPrint.printColTypes.push_back(table.coltypes[colIndex]);
        }

        // Read WHERE/ALL clause
        cin >> command;
        vector<uint32_t> rowsToPrint;

        if (command == "WHERE")
        {
            string colname, op, valueStr;
            cin >> colname >> op >> valueStr;

            // Validate column
            auto colIt = find(table.colnames.begin(), table.colnames.end(), colname);
            if (colIt == table.colnames.end())
            {
                cout << "Error during PRINT: " << colname << " does not name a column in " << printName << "\n";
                getline(cin, command);
                return;
            }
            size_t condColIndex = static_cast<size_t>(distance(table.colnames.begin(), colIt));
            ColumnType condColType = table.coltypes[condColIndex];

            // Convert value to correct type
            variant<int, double, bool, string> value;
            try
            {
                switch (condColType)
                {
                case ColumnType::Int:
                {
                    value = stoi(valueStr);
                    break;
                }
                case ColumnType::Double:
                {
                    value = stod(valueStr);
                    break;
                }
                case ColumnType::Bool:
                {
                    value = (valueStr == "true");
                    break;
                }
                case ColumnType::String:
                {
                    value = valueStr;
                    break;
                }
                }
            }
            catch (...)
            {
                cout << "Error: unrecognized command\n";
                getline(cin, command);
                return;
            }

            // Find matching rows using index if available
            if (table.indexedColumn == colname)
            {
                if (table.indexType == "hash")
                {
                    auto it = table.hashIndex.find(get<string>(value));
                    if (it != table.hashIndex.end())
                    {
                        rowsToPrint = it->second;
                    }
                }
                else if (table.indexType == "bst")
                {
                    auto it = table.bstIndex.find(value);
                    if (it != table.bstIndex.end())
                    {
                        rowsToPrint = it->second;
                    }
                }
            }
            else
            {
                // No index - linear search
                for (uint32_t i = 0; i < table.rows.size(); i++)
                {
                    auto &rowVal = table.rows[i][condColIndex];
                    bool match = false;

                    if (op == "=")
                    {
                        match = rowVal == value;
                    }
                    else if (op == "<")
                    {
                        match = rowVal < value;
                    }
                    else if (op == ">")
                    {
                        match = rowVal > value;
                    }

                    if (match)
                    {
                        rowsToPrint.push_back(i);
                    }
                }
            }
        }
        else if (command == "ALL")
        {
            // All rows in insertion order
            for (uint32_t i = 0; i < table.rows.size(); i++)
            {
                rowsToPrint.push_back(i);
            }
        }
        else
        {
            cout << "Error: unrecognized command\n";
            getline(cin, command);
            return;
        }

        // Print results
        if (!quiet)
        {
            // Print header
            for (uint32_t i = 0; i < columnsToPrint.printColNames.size(); i++)
            {
                if (i != 0)
                    cout << " ";
                cout << columnsToPrint.printColNames[i];
            }
            cout << "\n";

            // Print rows
            for (uint32_t rowIdx : rowsToPrint)
            {
                auto &row = table.rows[rowIdx];
                for (size_t colIdx = 0; colIdx < columnsToPrint.printColNames.size(); colIdx++)
                {
                    auto it = find(table.colnames.begin(), table.colnames.end(),
                                   columnsToPrint.printColNames[colIdx]);
                    size_t actualColIdx = static_cast<size_t>(distance(table.colnames.begin(), it));

                    if (colIdx != 0)
                        cout << " ";
                    visit([](const auto &v)
                          { cout << v; }, row[actualColIdx]);
                }
                cout << "\n";
            }
        }

        cout << "Printed " << rowsToPrint.size() << " matching rows from " << printName << "\n";
    }

    // DELETE implementation
    void deleteRow()
    {
        string tableName, colname, op, valueStr;
        cin >> command >> tableName >> command >> colname >> op >> valueStr;

        // Check table exists
        if (database.find(tableName) == database.end())
        {
            cout << "Error during DELETE: " << tableName << " does not name a table in the database\n";
            getline(cin, command);
            return;
        }

        Table &table = database[tableName];

        // Check column exists
        auto colIt = find(table.colnames.begin(), table.colnames.end(), colname);
        if (colIt == table.colnames.end())
        {
            cout << "Error during DELETE: " << colname << " does not name a column in " << tableName << "\n";
            getline(cin, command);
            return;
        }
        size_t condColIndex = static_cast<size_t>(distance(table.colnames.begin(), colIt));
        ColumnType condColType = table.coltypes[condColIndex];

        // Convert value to correct type
        variant<int, double, bool, string> value;
        try
        {
            switch (condColType)
            {
            case ColumnType::Int:
                value = stoi(valueStr);
                break;
            case ColumnType::Double:
                value = stod(valueStr);
                break;
            case ColumnType::Bool:
                value = (valueStr == "true");
                break;
            case ColumnType::String:
                value = valueStr;
                break;
            }
        }
        catch (...)
        {
            cout << "Error: unrecognized command\n";
            getline(cin, command);
            return;
        }

        // Find rows to delete
        vector<uint32_t> rowsToDelete;
        if (table.indexedColumn == colname)
        {
            if (table.indexType == "hash")
            {
                auto it = table.hashIndex.find(get<string>(value));
                if (it != table.hashIndex.end())
                {
                    rowsToDelete = it->second;
                }
            }
            else if (table.indexType == "bst")
            {
                auto it = table.bstIndex.find(value);
                if (it != table.bstIndex.end())
                {
                    rowsToDelete = it->second;
                }
            }
        }
        else
        {
            // Linear search
            for (uint32_t i = 0; i < table.rows.size(); i++)
            {
                auto &rowVal = table.rows[i][condColIndex];
                bool match = false;

                if (op == "=")
                    match = rowVal == value;
                else if (op == "<")
                    match = rowVal < value;
                else if (op == ">")
                    match = rowVal > value;

                if (match)
                    rowsToDelete.push_back(i);
            }
        }

        // Delete rows (from back to front to preserve indices)
        sort(rowsToDelete.rbegin(), rowsToDelete.rend());
        for (uint32_t rowIdx : rowsToDelete)
        {
            table.rows.erase(table.rows.begin() + rowIdx);
        }

        // Update index if needed
        if (!table.indexedColumn.empty())
        {
            buildIndex(table);
        }

        cout << "Deleted " << rowsToDelete.size() << " rows from " << tableName << "\n";
    }

    // JOIN implementation
    void join()
    {
        string table1, table2, col1, col2;
        cin >> table1 >> command >> table2 >> command >> col1 >> command >> col2 >> command;

        // Check tables exist
        if (database.find(table1) == database.end())
        {
            cout << "Error during JOIN: " << table1 << " does not name a table in the database\n";
            getline(cin, command);
            return;
        }
        if (database.find(table2) == database.end())
        {
            cout << "Error during JOIN: " << table2 << " does not name a table in the database\n";
            getline(cin, command);
            return;
        }

        Table &t1 = database[table1];
        Table &t2 = database[table2];

        // Check columns exist
        auto col1It = find(t1.colnames.begin(), t1.colnames.end(), col1);
        if (col1It == t1.colnames.end())
        {
            cout << "Error during JOIN: " << col1 << " does not name a column in " << table1 << "\n";
            getline(cin, command);
            return;
        }
        size_t col1Idx = static_cast<size_t>(distance(t1.colnames.begin(), col1It));

        auto col2It = find(t2.colnames.begin(), t2.colnames.end(), col2);
        if (col2It == t2.colnames.end())
        {
            cout << "Error during JOIN: " << col2 << " does not name a column in " << table2 << "\n";
            getline(cin, command);
            return;
        }
        size_t col2Idx = static_cast<size_t>(distance(t2.colnames.begin(), col2It));

        // Read PRINT columns
        uint32_t numPrintCols;
        cin >> command >> numPrintCols;

        vector<pair<string, uint32_t>> printColumns; // column name and table number (1 or 2)
        for (uint32_t i = 0; i < numPrintCols; i++)
        {
            string colName;
            uint32_t tableNum;
            cin >> colName >> tableNum;

            if (tableNum == 1)
            {
                auto it = find(t1.colnames.begin(), t1.colnames.end(), colName);
                if (it == t1.colnames.end())
                {
                    cout << "Error during JOIN: " << colName << " does not name a column in " << table1 << "\n";
                    getline(cin, command);
                    return;
                }
            }
            else if (tableNum == 2)
            {
                auto it = find(t2.colnames.begin(), t2.colnames.end(), colName);
                if (it == t2.colnames.end())
                {
                    cout << "Error during JOIN: " << colName << " does not name a column in " << table2 << "\n";
                    getline(cin, command);
                    return;
                }
            }
            else
            {
                cout << "Error during JOIN: invalid table number\n";
                getline(cin, command);
                return;
            }
            printColumns.emplace_back(colName, tableNum);
        }

        // Perform join
        vector<vector<string>> outputRows;

        for (auto &row1 : t1.rows)
        {
            auto &val1 = row1[col1Idx];

            for (auto &row2 : t2.rows)
            {
                auto &val2 = row2[col2Idx];

                if (val1 == val2)
                {
                    vector<string> outputRow;
                    for (auto &[colName, tableNum] : printColumns)
                    {
                        Table &t = (tableNum == 1) ? t1 : t2;
                        auto it = find(t.colnames.begin(), t.colnames.end(), colName);
                        size_t colIdx = static_cast<size_t>(distance(t.colnames.begin(), it));

                        auto &row = (tableNum == 1) ? row1 : row2;
                        string valStr;
                        visit([&](const auto &v)
                              { 
                        ostringstream oss;
                        oss << v;
                        valStr = oss.str(); }, row[colIdx]);
                        outputRow.push_back(valStr);
                    }
                    outputRows.push_back(outputRow);
                }
            }
        }

        // Print results
        if (!quiet)
        {
            // Print header
            for (uint32_t i = 0; i < printColumns.size(); i++)
            {
                if (i != 0)
                    cout << " ";
                cout << printColumns[i].first;
            }
            cout << "\n";

            // Print rows
            for (auto &row : outputRows)
            {
                for (uint32_t i = 0; i < row.size(); i++)
                {
                    if (i != 0)
                        cout << " ";
                    cout << row[i];
                }
                cout << "\n";
            }
        }

        cout << "Printed " << outputRows.size() << " rows from joining " << table1 << " to " << table2 << "\n";
    }

    // Helper function to build index
    void buildIndex(Table &table)
    {
        if (table.indexedColumn.empty())
            return;

        auto colIt = find(table.colnames.begin(), table.colnames.end(), table.indexedColumn);
        size_t colIdx = static_cast<size_t>(distance(table.colnames.begin(), colIt));

        if (table.indexType == "hash")
        {
            table.hashIndex.clear();
            for (uint32_t i = 0; i < table.rows.size(); i++)
            {
                string key = get<string>(table.rows[i][colIdx]);
                table.hashIndex[key].push_back(i);
            }
        }
        else if (table.indexType == "bst")
        {
            table.bstIndex.clear();
            for (uint32_t i = 0; i < table.rows.size(); i++)
            {
                auto &key = table.rows[i][colIdx];
                table.bstIndex[key].push_back(i);
            }
        }
    }

    // GENERATE implementation
    void userGenerate()
    {
        string tableName, indexType, colName;
        cin >> command >> tableName >> indexType >> command >> command >> colName;

        // Check table exists
        if (database.find(tableName) == database.end())
        {
            cout << "Error during GENERATE: " << tableName << " does not name a table in the database\n";
            getline(cin, command);
            return;
        }

        Table &table = database[tableName];

        // Check column exists
        auto colIt = find(table.colnames.begin(), table.colnames.end(), colName);
        if (colIt == table.colnames.end())
        {
            cout << "Error during GENERATE: " << colName << " does not name a column in " << tableName << "\n";
            getline(cin, command);
            return;
        }
        // size_t colIdx = distance(table.colnames.begin(), colIt);

        // Check index type is valid
        if (indexType != "hash" && indexType != "bst")
        {
            cout << "Error: unrecognized command\n";
            getline(cin, command);
            return;
        }

        // Clear old index and set new one
        table.indexedColumn = colName;
        table.indexType = indexType;
        table.hashIndex.clear();
        table.bstIndex.clear();

        // Build the index
        buildIndex(table);

        // Count distinct keys
        size_t distinctKeys = 0;
        if (indexType == "hash")
        {
            distinctKeys = table.hashIndex.size();
        }
        else
        {
            distinctKeys = table.bstIndex.size();
        }

        cout << "Generated " << indexType << " index for table " << tableName
             << " on column " << colName << ", with " << distinctKeys << " distinct keys\n";
    }

    void readIn()
    {
        do
        {
            if (cin.fail())
            {
                cerr << "Error: Reading from cin has failed" << endl;
                exit(1);
            }
            cout << "% ";
            cin >> command;

            if (command == "CREATE")
            {
                create();
            }
            else if (command == "QUIT")
            {
                cout << "Thanks for being silly!" << endl;
                return;
            }
            else if (command == "#")
            {
                getline(cin, command);
            }
            else if (command == "REMOVE")
            {
                remove();
            }
            else if (command == "INSERT")
            {
                insert();
            }
            else if (command == "PRINT")
            {
                print();
            }
            else if (command == "DELETE")
            {
                deleteRow();
            }
            else if (command == "JOIN")
            {
                join();
            }
            else if (command == "GENERATE")
            {
                userGenerate();
            }
            else {
                cout << "Error: unrecognized command" << endl;
            }
        } while (command != "QUIT");
    }
};

int main(int argc, char *argv[])
{
    ios_base::sync_with_stdio(false); // you should already have this
    cin >> std::boolalpha;            // add these two lines
    cout << std::boolalpha;

    Silly sillyQL;
    sillyQL.getMode(argc, argv);
    sillyQL.readIn();
    return 0;
}