// This program is designed to be a library management system.
// The system will allow the user to manage books, users, and borrowing transactions.
// Hugo Willis 
// CIS-17B-33477
// LMD: 03/09/25

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <exception>
using namespace std;

// Custom exception for library errors
class LibraryException : public exception {
private:
    string message;
public:
    LibraryException(const string &msg) : message(msg) {}
    virtual const char* what() const noexcept {
        return message.c_str();
    }
};

// Book class: Represents a book with book ID, title, author, and ISBN.
class Book {
private:
    static int nextBookID; 
    int bookID;
    string title;
    string author;
    string isbn;
    bool available;
public:
    Book(string t, string a, string i) {
        bookID = nextBookID++;
        title = t;
        author = a;
        isbn = i;
        available = true;
    }
    
    int getBookID() { return bookID; }
    string getTitle() { return title; }
    string getAuthor() { return author; }
    string getISBN() { return isbn; }
    bool isAvailable() { return available; }
    void setAvailable(bool avail) { available = avail; }
    
    void editBook(string newTitle, string newAuthor, string newISBN) {
        title = newTitle;
        author = newAuthor;
        isbn = newISBN;
    }
};

int Book::nextBookID = 0;  

// User class: Base class for users with an ID, name, and list of borrowed book IDs.
class User {
protected:
    static int nextUserID;  
    int userID;
    string name;
    vector<int> borrowedBooks;  
public:
    User(string n) {
        userID = nextUserID++;
        name = n;
    }
    virtual ~User() {}
    
    int getUserID() { return userID; }
    string getName() { return name; }
    
    // Virtual function so derived classes can indicate type (Student/Faculty)
    virtual string getUserType() = 0;
    
    // Maximum books allowed (default 3)
    virtual int getMaxBooks() { return 3; }
    
    bool canBorrow() { return borrowedBooks.size() < static_cast<unsigned>(getMaxBooks()); }
    
    void borrowBook(int bookID) {
        borrowedBooks.push_back(bookID);
    }
    
    void returnBook(int bookID) {
        bool found = false;
        for (size_t i = 0; i < borrowedBooks.size(); i++) {
            if (borrowedBooks[i] == bookID) {
                borrowedBooks.erase(borrowedBooks.begin() + i);
                found = true;
                break;
            }
        }
        if (!found)
            throw LibraryException("Book not borrowed by user.");
    }
    
    const vector<int>& getBorrowedBooks() { return borrowedBooks; }
    
    void editUser(string newName) {
        name = newName;
    }
};

int User::nextUserID = 0;  

// Derived class: Student
class Student : public User {
public:
    Student(string n) : User(n) {}
    string getUserType() { return "Student"; }
    int getMaxBooks() { return 3; }
};

// Derived class: Faculty
class Faculty : public User {
public:
    Faculty(string n) : User(n) {}
    string getUserType() { return "Faculty"; }
    int getMaxBooks() { return 5; }
};

// Factory for creating Book objects
class BookFactory {
public:
    static Book* createBook(string title, string author, string isbn) {
        return new Book(title, author, isbn);
    }
};

// Factory for creating User objects
class UserFactory {
public:
    static User* createUser(int userType, string name) {
        if (userType == 1)
            return new Student(name);
        else if (userType == 2)
            return new Faculty(name);
        else
            throw LibraryException("Only valid options are 1 or 2");
    }
};

// Singleton that manages Books and Users, and handles transactions.
class Library {
private:
    vector<Book*> books;    
    vector<User*> users;   

    Library() {}  

    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;
public:
    static Library& getInstance() {
        static Library instance;
        return instance;
    }
    
    // Book management
    void addBook(Book* book) {
        books.push_back(book);
    }
    
    Book* getBook(int bookID) {
        for (size_t i = 0; i < books.size(); i++) {
            Book* b = books[i];
            if (b->getBookID() == bookID)
                return b;
        }
        return nullptr;
    }
    
    void editBook(int bookID, string newTitle, string newAuthor, string newISBN) {
        Book* book = getBook(bookID);
        if (!book)
            throw LibraryException("Book not found.");
        book->editBook(newTitle, newAuthor, newISBN);
    }
    
    void removeBook(int bookID) {
        for (vector<Book*>::iterator it = books.begin(); it != books.end(); ++it) {
            if ((*it)->getBookID() == bookID) {
                delete *it;
                books.erase(it);
                return;
            }
        }
        throw LibraryException("Book not found.");
    }
    
    // Find a book by title 
    Book* findBookByTitle(string title) {
        for (size_t i = 0; i < books.size(); i++) {
            Book* b = books[i];
            if (b->getTitle() == title)
                return b;
        }
        return nullptr;
    }
    
    // User management
    void registerUser(User* user) {
        users.push_back(user);
    }
    
    User* getUser(int userID) {
        for (size_t i = 0; i < users.size(); i++) {
            User* u = users[i];
            if (u->getUserID() == userID)
                return u;
        }
        return nullptr;
    }
    
    void editUser(int userID, string newName) {
        User* user = getUser(userID);
        if (!user)
            throw LibraryException("User not found.");
        user->editUser(newName);
    }
    
    void removeUser(int userID) {
        for (vector<User*>::iterator it = users.begin(); it != users.end(); ++it) {
            if ((*it)->getUserID() == userID) {
                delete *it;
                users.erase(it);
                return;
            }
        }
        throw LibraryException("User not found.");
    }
    
    // Borrow a book (by user and book IDs)
    void borrowBook(int userID, int bookID) {
        User* user = getUser(userID);
        if (!user)
            throw LibraryException("No User with that ID Exists");
        Book* book = getBook(bookID);
        if (!book)
            throw LibraryException("No Book with that ID Exists");
        if (!book->isAvailable())
            throw LibraryException("Book is not available for borrowing.");
        if (!user->canBorrow())
            throw LibraryException("User has reached borrowing limit.");
        
        book->setAvailable(false);
        user->borrowBook(bookID);
    }
    
    // Return a book (by user and book IDs)
    void returnBook(int userID, int bookID) {
        User* user = getUser(userID);
        if (!user)
            throw LibraryException("No User with that ID Exists");
        Book* book = getBook(bookID);
        if (!book)
            throw LibraryException("No Book with that ID Exists");
        
        user->returnBook(bookID);
        book->setAvailable(true);
    }
    
    // List all books with details 
    void listAllBooks() {
        cout << "List All Books" << endl;
        for (size_t i = 0; i < books.size(); i++) {
            Book* b = books[i];
            cout << "Book " << b->getBookID() << ":" << endl;
            cout << "Title: " << b->getTitle() << endl;
            cout << "Author: " << b->getAuthor() << endl;
            cout << "ISBN: " << b->getISBN() << endl;
        }
    }
    
    // List all users with their borrowed books.
    void listAllUsers() {
        cout << "List All Users" << endl;
        for (size_t i = 0; i < users.size(); i++) {
            User* u = users[i];
            cout << "User " << u->getUserID() << ":" << endl;
            cout << "Name: " << u->getName() << endl;
            cout << "Class: " << u->getUserType() << endl;
            cout << "Books Checked Out:" << endl;
            const vector<int>& borrowed = u->getBorrowedBooks();
            for (size_t j = 0; j < borrowed.size(); j++) {
                int bid = borrowed[j];
                Book* b = getBook(bid);
                if (b) {
                    cout << "Book " << b->getBookID() << ":" << endl;
                    cout << "Title: " << b->getTitle() << endl;
                    cout << "Author: " << b->getAuthor() << endl;
                    cout << "ISBN: " << b->getISBN() << endl;
                }
            }
        }
    }
    
    ~Library() {
        for (size_t i = 0; i < books.size(); i++) {
            delete books[i];
        }
        for (size_t i = 0; i < users.size(); i++) {
            delete users[i];
        }
    }
};

void clearInput() {
    cin.clear();
    cin.ignore(10000, '\n');
}

int main() {
    Library &library = Library::getInstance();
    int choice;
    
    while (true) {
        cout << "Welcome to the Norco Library:" << endl;
        cout << "1. Manage Books" << endl;
        cout << "2. Manage Users" << endl;
        cout << "3. Manage Transactions" << endl;
        cout << "4. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        clearInput();
        
        if (choice == 1) {
            // Manage Books submenu 
            int bookChoice;
            while (true) {
                cout << "\nManage Books:" << endl;
                cout << "1. Add a Book" << endl;
                cout << "2. Edit a Book" << endl;
                cout << "3. Remove a Book" << endl;
                cout << "4. Go Back" << endl;
                cout << "\nEnter your choice: ";
                cin >> bookChoice;
                clearInput();
                
                if (bookChoice == 1) {
                    string title, author, isbn;
                    cout << "\nAdd a Book:" << endl;
                    cout << "Enter the Title (0 to cancel): ";
                    getline(cin, title);
                    if (title == "0") continue;
                    cout << "Enter the Author (0 to cancel): ";
                    getline(cin, author);
                    if (author == "0") continue;
                    cout << "Enter the ISBN (0 to cancel): ";
                    getline(cin, isbn);
                    if (isbn == "0") continue;
                    Book* newBook = BookFactory::createBook(title, author, isbn);
                    library.addBook(newBook);
                    cout << "Book Added" << endl;
                }
                else if (bookChoice == 2) {
                    int bookID;
                    cout << "\nEdit a Book:" << endl;
                    cout << "Enter Book ID to edit (or -1 to cancel): ";
                    cin >> bookID;
                    clearInput();
                    if (bookID == -1) continue;
                    if (!library.getBook(bookID)) {
                        cout << "ERROR: Book not found" << endl;
                        continue;
                    }
                    string newTitle, newAuthor, newISBN;
                    cout << "Enter new Title: ";
                    getline(cin, newTitle);
                    cout << "Enter new Author: ";
                    getline(cin, newAuthor);
                    cout << "Enter new ISBN: ";
                    getline(cin, newISBN);
                    try {
                        library.editBook(bookID, newTitle, newAuthor, newISBN);
                        cout << "Book Edited" << endl;
                    }
                    catch (LibraryException &e) {
                        cout << "ERROR: " << e.what() << endl;
                    }
                }
                else if (bookChoice == 3) {
                    int bookID;
                    cout << "\nRemove a Book:" << endl;
                    cout << "Enter Book ID to remove (or -1 to cancel): ";
                    cin >> bookID;
                    clearInput();
                    if (bookID == -1) continue;
                    try {
                        library.removeBook(bookID);
                        cout << "Book Removed" << endl;
                    }
                    catch (LibraryException &e) {
                        cout << "ERROR: " << e.what() << endl;
                    }
                }
                else if (bookChoice == 4) {
                    break;
                }
                else {
                    cout << "ERROR: Invalid choice" << endl;
                }
            }
        }
        else if (choice == 2) {
            // Manage Users submenu 
            int userChoice;
            while (true) {
                cout << "\nManage Users:" << endl;
                cout << "1. Add a User" << endl;
                cout << "2. Edit a User" << endl;
                cout << "3. Remove a User" << endl;
                cout << "4. Go Back" << endl;
                cout << "\nEnter your choice: ";
                cin >> userChoice;
                clearInput();
                
                if (userChoice == 1) {
                    int userType;
                    string name;
                    cout << "Add a User:" << endl;
                    while (true) {
                        cout << "Enter 1 for student or 2 for faculty (0 to cancel): ";
                        cin >> userType;
                        clearInput();
                        if (userType == 0)
                            break;
                        if (userType != 1 && userType != 2) {
                            cout << "ERROR: Only valid options are 1 or 2" << endl;
                        } else {
                            cout << "Enter name (0 to cancel): ";
                            getline(cin, name);
                            if (name == "0")
                                break;
                            try {
                                User* newUser = UserFactory::createUser(userType, name);
                                library.registerUser(newUser);
                                cout << "User Added with ID " << newUser->getUserID() << endl;
                            }
                            catch (LibraryException &e) {
                                cout << "ERROR: " << e.what() << endl;
                            }
                            break;
                        }
                    }
                }
                else if (userChoice == 2) {
                    int userID;
                    string newName;
                    cout << "Edit a User:" << endl;
                    cout << "Enter User ID to edit (or -1 to cancel): ";
                    cin >> userID;
                    clearInput();
                    if (userID == -1) continue;
                    if (!library.getUser(userID)) {
                        cout << "ERROR: User not found" << endl;
                        continue;
                    }
                    cout << "Enter new name: ";
                    getline(cin, newName);
                    try {
                        library.editUser(userID, newName);
                        cout << "User Edited" << endl;
                    }
                    catch (LibraryException &e) {
                        cout << "ERROR: " << e.what() << endl;
                    }
                }
                else if (userChoice == 3) {
                    int userID;
                    cout << "Remove a User:" << endl;
                    cout << "Enter User ID to remove (or -1 to cancel): ";
                    cin >> userID;
                    clearInput();
                    if (userID == -1) continue;
                    try {
                        library.removeUser(userID);
                        cout << "User Removed" << endl;
                    }
                    catch (LibraryException &e) {
                        cout << "ERROR: " << e.what() << endl;
                    }
                }
                else if (userChoice == 4) {
                    break;
                }
                else {
                    cout << "ERROR: Invalid choice" << endl;
                }
            }
        }
        else if (choice == 3) {
            // Manage Transactions submenu 
            int transChoice;
            while (true) {
                cout << "\nManage Transactions:" << endl;
                cout << "1. Check Out A Book" << endl;
                cout << "2. Check In A Book" << endl;
                cout << "3. List All Books" << endl;
                cout << "4. List All Users" << endl;
                cout << "5. Go Back" << endl;
                cout << "\nEnter your choice: ";
                cin >> transChoice;
                clearInput();
                
                if (transChoice == 1) {
                    string bookTitle;
                    int userID;
                    Book* book = nullptr;
                    cout << "\nCheck Out A Book:" << endl;
                    while (true) {
                        cout << "Book Title (or 0 to cancel): ";
                        getline(cin, bookTitle);
                        if (bookTitle == "0")
                            break;
                        book = library.findBookByTitle(bookTitle);
                        if (!book)
                            cout << "Error: No book with that title exists" << endl;
                        else
                            break;
                    }
                    if (book == nullptr)
                        continue;
                    
                    while (true) {
                        cout << "User ID (or x to cancel): ";
                        string userInput;
                        getline(cin, userInput);
                        if (userInput == "x" || userInput == "X")
                            break;
                        try {
                            userID = stoi(userInput);
                        }
                        catch (...) {
                            cout << "Error: Invalid User ID" << endl;
                            continue;
                        }
                        if (!library.getUser(userID)) {
                            cout << "Error: No User with that ID Exists" << endl;
                            continue;
                        }
                        try {
                            library.borrowBook(userID, book->getBookID());
                            cout << book->getTitle() << " checked out by User " << userID << endl;
                        }
                        catch (LibraryException &e) {
                            cout << "Error: " << e.what() << endl;
                        }
                        break;
                    }
                }
                else if (transChoice == 2) {
                    string bookTitle;
                    int userID;
                    Book* book = nullptr;
                    cout << "\nCheck In A Book:" << endl;
                    while (true) {
                        cout << "Book Title (or 0 to cancel): ";
                        getline(cin, bookTitle);
                        if (bookTitle == "0")
                            break;
                        book = library.findBookByTitle(bookTitle);
                        if (!book)
                            cout << "Error: No book with that title exists" << endl;
                        else
                            break;
                    }
                    if (book == nullptr)
                        continue;
                    
                    while (true) {
                        cout << "User ID (or x to cancel): ";
                        string userInput;
                        getline(cin, userInput);
                        if (userInput == "x" || userInput == "X")
                            break;
                        try {
                            userID = stoi(userInput);
                        }
                        catch (...) {
                            cout << "Error: Invalid User ID" << endl;
                            continue;
                        }
                        if (!library.getUser(userID)) {
                            cout << "Error: No User with that ID Exists" << endl;
                            continue;
                        }
                        try {
                            library.returnBook(userID, book->getBookID());
                            cout << book->getTitle() << " checked in by User " << userID << endl;
                        }
                        catch (LibraryException &e) {
                            cout << "Error: " << e.what() << endl;
                        }
                        break;
                    }
                }
                else if (transChoice == 3) {
                    library.listAllBooks();
                }
                else if (transChoice == 4) {
                    library.listAllUsers();
                }
                else if (transChoice == 5) {
                    break;
                }
                else {
                    cout << "ERROR: Invalid choice" << endl;
                }
            }
        }
        else if (choice == 4) {
            cout << "Thank you for using the Library System!" << endl;
            break;
        }
        else {
            cout << "ERROR: Invalid choice, please try again" << endl;
        }
    }
    
    return 0;
}
