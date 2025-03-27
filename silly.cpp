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

#include "Field.h"
#include "functor.h"

using namespace std;

// CREATE <tablename> <N> <coltype1> <coltype2> ... <coltypeN> <colname1> <colname2> ... <colnameN>
// N = columns (> 0), each column contains coltype with colname
// valid coltypes are {double, int, bool, string}

// enum class ColumnType { String, Int, Double, Bool };

struct Table
{
    uint16_t N;              // Number of columns
    string tablename;        // Table name
    vector<string> colnames; // Column names
    vector<ColumnType> coltypes;
    unordered_map<string, ColumnType> fullColumns; // Column name -> Column type mapping
};

class Silly
{
public:
    /*****************VARIABLES********************/
    bool quiet = false;
    string cmd;
    unordered_map<string, Table> database;

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
                cout << "Usage of Commands:" << endl;
                cout << "CREATE: Creates a new table with <N> columns (where N > 0)." << endl;
                cout << "INSERT INTO: Inserts <N> new rows (where N > 0) into the table specified by <tablename>. " << endl;
                cout << "DELETE FROM: Deletes all rows from the table specified by <tablename> where the value of "
                     << "the entry in <colname> satisfies the operation <OP> with the given value <value>. " << endl;
                cout << "GENERATE INDEX: Directs the program to create an index of the type "
                     << "<indextype> on the column <colname> in the table< tablename > " << endl;
                cout << "PRINT: Directs the program to print the columns specified by <print_colname1>, <print_colname2>, ..."
                     << "< print_colnameN > from some / all rows in <tablename>. " << endl;
                cout << "JOIN: Directs the program to print the data in <N> columns, specified by <print_colname1>,"
                     << "< print_colname2 > , ... <print_colnameN>." << endl;
                cout << "REMOVE: Removes the table specified by <tablename> and all associated data from the database, including any created index." << endl;
                cout << "QUIT: Cleans up all internal data (i.e. no memory leaks) and exits the program. " << endl;
                exit(0);
                break;
            case 'q':
                quiet = true;
                break;
            default:
                cerr << "Error: Unknown command line option" << endl;
                exit(1);
            } // switch
        } // while
    }

    /******************COMMANDS********************/
    // CREATE <tablename> <N> <coltype1> <coltype2> ... <coltypeN> <colname1> <colname2> ... <colnameN>
    // N = columns (> 0), each column contains coltype with colname
    // valid colltypes are {double, int, bool, string}
    void create()
    {
        Table creation;
        cin >> creation.tablename >> creation.N; // Read table name and number of columns

        creation.colnames.resize(creation.N);
        creation.coltypes.resize(creation.N);
        // creation.fullColumns.resize(creation.N);

        // First loop: Read and store column types
        if (database.find(creation.tablename) == database.end())
        {
            for (uint16_t i = 0; i < creation.N; ++i)
            {
                string coltype;
                cin >> coltype; // Read column type

                // Map the string type to ColumnType
                if (coltype.at(0) == 's')
                {
                    creation.coltypes[i] = ColumnType::String;
                }
                else if (coltype.at(0) == 'i')
                {
                    creation.coltypes[i] = ColumnType::Int;
                }
                else if (coltype.at(0) == 'd')
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
            getline(cin, cmd);
            return;
        }
    }

    void remove(){
        string erase;
        cin >> erase;

        if (database.find(erase) != database.end()){
            database.erase(erase);
            cout << "Table " << erase << " removed\n";
        }
        else{
            cout << "Error during REMOVE: " << erase << " does not name a table in the database\n";
        }
    }
    

    void readIn()
    {
        string command;

        do
        {
            if (cin.fail())
            {
                cerr << "Error: Reading from cin has failed" << endl;
                exit(1);
            }
            cout << "% ";
            cin >> command;

            if(command == "CREATE")
            {
                create();
            }
            else if(command == "QUIT"){
                return;
            }
            else if(command.at(0) == '#') {
				getline(cin, command);
			}
            else if(command == "REMOVE"){
                remove();
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
    cout << "Thanks for being silly!" << endl;
    return 0;
}