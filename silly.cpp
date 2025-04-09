// PROJECT IDENTIFIER: C0F4DFE8B340D81183C208F70F9D2D797908754D

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <unordered_map>
#include <getopt.h>
#include <sstream>
#include <variant>

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

    unordered_map<variant<int, double, bool, string>, vector<uint32_t>> hashIndex; // hash index
    map<variant<int, double, bool, string>, vector<uint32_t>> bstIndex;            // bst index
    string indexedColumn;                                                          // name of column that's indexed
    string indexType;                                                              // "hash" or "bst" or empty
};

struct printTable
{
    vector<ColumnType> printColTypes;
    vector<string> printColNames;
};

class Silly
{
public:
    bool quiet = false;
    string errorJunk;
    unordered_map<string, Table> database;
    string command;

    void printHelp(char *command)
    {
        cout << "Usage: " << command << "  -h | -q\n";
    } // printHelp()

    void getMode(int argc, char *argv[])
    {
        opterr = false;
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

            for (uint16_t i = 0; i < creation.N; ++i)
            {
                string colname;
                cin >> colname;

                creation.colnames[i] = colname;
            }

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
        string junk;

        cin >> junk >> insertTable >> rows >> junk;

        if (database.find(insertTable) != database.end())
        {
            database[insertTable].rows.reserve(database[insertTable].rows.size() + rows);

            for (uint32_t i = 0; i < rows; i++)
            {
                vector<variant<int, double, bool, string>> newRow;

                for (uint32_t j = 0; j < database[insertTable].colnames.size(); j++)
                {
                    string value;
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

                database[insertTable].rows.push_back(newRow);
            }
        }

        else
        {
            cout << "Error during INSERT: " << insertTable << " does not name a table in the database\n";
            getline(cin, errorJunk);
            return;
        }

        cout << "Added " << rows << " rows to " << database[insertTable].tablename << " from position " << database[insertTable].currentIndex << " to ";
        database[insertTable].currentIndex += rows;
        cout << database[insertTable].currentIndex - 1 << "\n";

        buildIndex(database[insertTable]);
    }

    void print()
    {
        string printName;
        uint32_t numCols;
        cin >> command >> printName >> numCols;

        if (database.find(printName) == database.end())
        {
            cout << "Error during PRINT: " << printName << " does not name a table in the database\n";
            getline(cin, command);
            return;
        }

        Table &table = database[printName];
        printTable columnsToPrint;

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

        cin >> command;
        vector<uint32_t> rowsToPrint;

        if (command == "WHERE")
        {
            string colname, op, valueStr;
            cin >> colname >> op >> valueStr;

            auto colIt = find(table.colnames.begin(), table.colnames.end(), colname);
            if (colIt == table.colnames.end())
            {
                cout << "Error during PRINT: " << colname << " does not name a column in " << printName << "\n";
                getline(cin, command);
                return;
            }
            size_t condColIndex = static_cast<size_t>(distance(table.colnames.begin(), colIt));
            ColumnType condColType = table.coltypes[condColIndex];

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
                return;
            }

            if (table.indexedColumn == colname)
            {
                if (table.indexType == "hash" && op == "=")
                {
                    auto it = table.hashIndex.find(value);
                    if (it != table.hashIndex.end())
                    {
                        rowsToPrint = it->second;
                    }

                    sort(rowsToPrint.begin(), rowsToPrint.end());
                }

                else if (table.indexType == "bst")
                {
                    if (op == "=")
                    {
                        auto it = table.bstIndex.find(value);
                        if (it != table.bstIndex.end())
                        {
                            rowsToPrint = it->second;
                        }
                    }
                    else if (op == ">")
                    {
                        for (auto it = table.bstIndex.upper_bound(value);
                             it != table.bstIndex.end(); ++it)
                        {
                            rowsToPrint.insert(rowsToPrint.end(),
                                               it->second.begin(), it->second.end());
                        }
                    }
                    else if (op == "<")
                    {
                        for (auto it = table.bstIndex.begin();
                             it != table.bstIndex.lower_bound(value); ++it)
                        {
                            rowsToPrint.insert(rowsToPrint.end(),
                                               it->second.begin(), it->second.end());
                        }
                    }
                }
                else
                {
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
            else
            {
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
            for (uint32_t i = 0; i < table.rows.size(); i++)
            {
                rowsToPrint.push_back(i);
            }
        }

        if (!quiet)
        {
            for (uint32_t i = 0; i < columnsToPrint.printColNames.size(); i++)
            {
                cout << columnsToPrint.printColNames[i];
                cout << " ";
            }
            cout << "\n";

            for (uint32_t rowIdx : rowsToPrint)
            {
                const auto &row = table.rows[rowIdx];

                for (size_t colIdx = 0; colIdx < columnsToPrint.printColNames.size(); colIdx++)
                {
                    auto it = find(table.colnames.begin(), table.colnames.end(),
                                   columnsToPrint.printColNames[colIdx]);
                    size_t actualColIdx = static_cast<size_t>(distance(table.colnames.begin(), it));

                    visit([](const auto &val)
                          { cout << val << " "; }, row[actualColIdx]);
                }

                cout << "\n";
            }
        }

        cout << "Printed " << rowsToPrint.size() << " matching rows from " << printName << "\n";
    }

    void deleteRow()
    {
        string tableName;
        string colname;
        string op;
        string valueStr;
        cin >> command >> tableName >> command >> colname >> op >> valueStr;

        if (database.find(tableName) == database.end())
        {
            cout << "Error during DELETE: " << tableName << " does not name a table in the database\n";
            getline(cin, command);
            return;
        }

        Table &table = database[tableName];

        auto colIt = find(table.colnames.begin(), table.colnames.end(), colname);
        if (colIt == table.colnames.end())
        {
            cout << "Error during DELETE: " << colname << " does not name a column in " << tableName << "\n";
            getline(cin, command);
            return;
        }
        size_t condColIndex = static_cast<size_t>(distance(table.colnames.begin(), colIt));
        ColumnType condColType = table.coltypes[condColIndex];

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
            return;
        }

        vector<uint32_t> rowsToDelete;
        if (table.indexedColumn == colname)
        {
            if (table.indexType == "hash" && op == "=")
            {

                auto it = table.hashIndex.find(value);
                if (it != table.hashIndex.end())
                {
                    rowsToDelete = it->second;
                }
                sort(rowsToDelete.begin(), rowsToDelete.end());
            }
            // sort(rowsToDelete.begin(), rowsToDelete.end());

            else if (table.indexType == "bst")
            {
                if (op == "=")
                {
                    auto it = table.bstIndex.find(value);
                    if (it != table.bstIndex.end())
                    {
                        rowsToDelete = it->second;
                    }
                }
                else if (op == ">")
                {
                    for (auto it = table.bstIndex.upper_bound(value);
                         it != table.bstIndex.end(); ++it)
                    {
                        rowsToDelete.insert(rowsToDelete.end(),
                                            it->second.begin(), it->second.end());
                    }
                }
                else if (op == "<")
                {
                    for (auto it = table.bstIndex.begin();
                         it != table.bstIndex.lower_bound(value); ++it)
                    {
                        rowsToDelete.insert(rowsToDelete.end(),
                                            it->second.begin(), it->second.end());
                    }
                }
            }

            else
            {
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
                        rowsToDelete.push_back(i);
                    }
                }
            }
        }

        else
        {
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
                    rowsToDelete.push_back(i);
                }
            }
        }

        sort(rowsToDelete.rbegin(), rowsToDelete.rend());

        for (uint32_t rowIdx : rowsToDelete)
        {
            table.rows.erase(table.rows.begin() + rowIdx);
        }

        buildIndex(table);

        cout << "Deleted " << rowsToDelete.size() << " rows from " << tableName << "\n";
        database[tableName].currentIndex -= static_cast<uint32_t>(rowsToDelete.size());
    }

    void join()
    {
        string table1;
        string table2;
        string col1;
        string col2;
        cin >> table1 >> command >> table2 >> command >> col1 >> command >> col2 >> command;

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
        //buildIndex(t2);

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

        uint32_t numPrintCols;
        cin >> command >> numPrintCols;

        vector<pair<string, uint32_t>> printColumns;
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
            printColumns.emplace_back(colName, tableNum);
        }

        vector<vector<string>> outputRows;
        unordered_map<variant<int, double, bool, string>, vector<uint32_t>> tempHash; // Key: join value, Value: row indices in t2

        for (uint32_t i = 0; i < t2.rows.size(); ++i)
            {
                const auto &val = t2.rows[i][col2Idx];
                string key = visit([](const auto &val) -> string
                                   {
            using T = decay_t<decltype(val)>;
            if constexpr (is_same_v<T, int>) {
                return to_string(val);
            }
            else if constexpr (is_same_v<T, double>) {
                string s = to_string(val);
                
                s.erase(s.find_last_not_of('0') + 1, string::npos);
                
                if (!s.empty() && s.back() == '.') {
                    s.pop_back();
                }
                
                return s;
            }
            else if constexpr (is_same_v<T, bool>) return val ? "true" : "false";
            else if constexpr (is_same_v<T, string>) return val;
            return ""; }, val);
                tempHash[key].push_back(i);
            }

        for (const auto &row1 : t1.rows)
            {
                const auto &val1 = row1[col1Idx];

                string key = visit([](const auto &val) -> string
                                   {
            using T = decay_t<decltype(val)>;
            if constexpr (is_same_v<T, int>) {
                return to_string(val);
            }
            else if constexpr (is_same_v<T, double>) {
                string s = to_string(val);
                
                s.erase(s.find_last_not_of('0') + 1, string::npos);
                
                if (!s.empty() && s.back() == '.') {
                    s.pop_back();
                }
                
                return s;
            }
            else if constexpr (is_same_v<T, bool>) {
                return val ? "true" : "false";
            }
            else if constexpr (is_same_v<T, string>) {
                return val;
            }
            return ""; }, val1);

                if (tempHash.find(key) != tempHash.end())
                {
                    for (uint32_t row2Idx : tempHash.at(key))
                    {
                        const auto &row2 = t2.rows[row2Idx];
                        vector<string> outputRow;

                        for (const auto &[colName, tableNum] : printColumns)
                        {
                            const Table &t = (tableNum == 1) ? t1 : t2;

                            size_t colIdx = 0;
                            for (; colIdx < t.colnames.size(); ++colIdx)
                            {
                                if (t.colnames[colIdx] == colName)
                                    break;
                            }

                            const auto &cell = (tableNum == 1) ? row1[colIdx] : row2[colIdx];
                            string valStr = visit([](const auto &val) -> string
                                                  {
                        using T = decay_t<decltype(val)>;
                        if constexpr (is_same_v<T, int>) {
                            return to_string(val);
                        }
                        else if constexpr (is_same_v<T, double>) {
                            string s = to_string(val);
                            
                            s.erase(s.find_last_not_of('0') + 1, string::npos);
                            
                            if (!s.empty() && s.back() == '.') {
                                s.pop_back();
                            }
                            
                            return s;
                        }
                        else if constexpr (is_same_v<T, bool>) {
                            return val ? "true" : "false";
                        }
                        else if constexpr (is_same_v<T, string>) {
                            return val;
                        }
                        return ""; }, cell);

                            outputRow.push_back(valStr);
                        }

                        outputRows.push_back(outputRow);
                    }
                }
            }

        if (!quiet)
        {
            for (uint32_t i = 0; i < printColumns.size(); i++)
            {
                cout << printColumns[i].first << " ";
            }
            cout << "\n";

            for (auto &row : outputRows)
            {
                for (uint32_t i = 0; i < row.size(); i++)
                {
                    cout << row[i] << " ";
                }
                cout << "\n";
            }
        }

        buildIndex(t2);

        cout << "Printed " << outputRows.size() << " rows from joining " << table1 << " to " << table2 << "\n";
    }

    void buildIndex(Table &table)
    {
        table.hashIndex.clear();
        table.bstIndex.clear();

        if (table.indexedColumn.empty())
                return;

        auto colIt = find(table.colnames.begin(), table.colnames.end(), table.indexedColumn);
        size_t colIdx = static_cast<size_t>(distance(table.colnames.begin(), colIt));

        if (table.indexType == "hash")
        {
            for (uint32_t i = 0; i < table.rows.size(); i++)
            {
                auto &key = table.rows[i][colIdx];
                table.hashIndex[key].push_back(i);
            }
        }
        else if (table.indexType == "bst")
        {
            for (uint32_t i = 0; i < table.rows.size(); i++)
            {
                auto &key = table.rows[i][colIdx];
                table.bstIndex[key].push_back(i);
            }
        }
    }

    void userGenerate()
    {
        string tableName;
        string indexType;
        string colName;
        cin >> command >> tableName >> indexType >> command >> command >> colName;

        if (database.find(tableName) == database.end())
        {
            cout << "Error during GENERATE: " << tableName << " does not name a table in the database\n";
            getline(cin, command);
            return;
        }

        Table &table = database[tableName];

        auto colIt = find(table.colnames.begin(), table.colnames.end(), colName);
        if (colIt == table.colnames.end())
        {
            cout << "Error during GENERATE: " << colName << " does not name a column in " << tableName << "\n";
            getline(cin, command);
            return;
        }

        table.indexedColumn = colName;
        table.indexType = indexType;

        buildIndex(table);

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
            else if (command.at(0) == '#')
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
            else
            {
                cout << "Error: unrecognized command" << endl;
                getline(cin, command);
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