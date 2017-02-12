/**
 * CS 106B/X Stanford 1-2-3
 * This file defines the interface for Stanford123Gui class used in the
 * Stanford 1-2-3 spreadsheet program.
 *
 * Please do not modify this provided file. Your turned-in files should work
 * with an unmodified version of all provided code files.
 *
 * @author Marty Stepp, Julie Zelenski, Jerry Cain
 * @version 2016/11/29
 * - 16au 106x version (stepp)
 */

#ifndef _stanford123gui_h
#define _stanford123gui_h

#include <string>
#include "gevents.h"
#include "ginteractors.h"
#include "gtable.h"
#include "gwindow.h"
#include "spreadsheet.h"
#include "view.h"

/**
 * This class handles the display of a spreadsheet in the graphics
 * window. It exports two public member functions, one to display the
 * spreadsheet grid/labels/background, and another to display the
 * contents for a given cell. The member functions are intended to
 * be invoked by the model when cells are updated.
 */
class Stanford123Gui : public View {
public:
    /**
     * The constructor initializes the graphics window, configures
     * the view object, and displays an empty spreadsheet.
     */
    Stanford123Gui(Spreadsheet* model = nullptr);

    /**
     * This member function clears out the contents of all cells.
     * Your spreadsheet should call this when your clear() method is called.
     */
    virtual void clearCells();

    /**
     * This member function draws the contents for a given cell.
     * Your spreadsheet should call this when your setCell() method is called.
     * You can pass either a 0-based row/column as integers or a cell name
     * in the format of a column letter followed by row number, e.g. "A7".
     * In this format, columns are lettered starting from 'A and rows are numbered
     * starting from 1.
     */
    virtual void displayCell(int row, int column, const std::string& text);
    virtual void displayCell(const std::string& cellname, const std::string& text);

    /**
     * Starts a main loop to process graphical events that occur in the window.
     * This method will call processXxxEvent as each event comes in.
     * Not to be called by students.
     */
    void eventLoop();

    /**
     * Sets this GUI to read its data from the given spreadsheet model.
     */
    void setModel(Spreadsheet* model);

private:
    /**
     * Initiates a clear action.
     */
    void clear();

    /**
     * Removes any text in the bottom status bar area.
     */
    void clearStatusMessage();

    /**
     * Returns true if the current spreadsheet document has been modified since
     * it was last saved.
     */
    bool isDocumentModified() const;

    /**
     * Initiates a load action.
     */
    void load();

    /**
     * Handles one action event in the window's event loop.
     */
    void processActionEvent(GActionEvent& actionEvent);

    /**
     * Handles one keyboard event in the window's event loop.
     */
    void processKeyEvent(GKeyEvent& keyEvent);

    /**
     * Handles one table event in the window's event loop.
     */
    void processTableEvent(GTableEvent& tableEvent);

    /**
     * Handles one window event in the window's event loop.
     */
    bool processWindowEvent(GWindowEvent& windowEvent);

    /**
     * Initiates a save action.
     */
    void save();

    /**
     * Sets the value that should be shown in the current cell editor window.
     * This is used to show a cell's formula while you are editing it.
     */
    void setCellEditorValue(const std::string& cellname, const std::string& txt);

    /**
     * Sets whether the current document has been modified since it was last
     * saved. This is shown via a * (asterisk) in the window title bar.
     */
    void setDocumentModified(bool modified = true);

    /**
     * Sets the status message text to show in the window's bottom status bar.
     * If isError is true, shows the text in a reddish error color.
     */
    void setStatusMessage(const std::string& message, bool isError = false);

    /**
     * Sets the text to display in the top formula display field.
     */
    void updateFormulaFieldText();

private:
    /**
     * Constants that control the number of visible rows/cols
     * in the graphics window.
     */
    static const int ROWS_TO_DISPLAY_COUNT = 20;
    static const int COLS_TO_DISPLAY_COUNT = 10;
    static const constexpr double COL_WIDTH = 75;
    static const constexpr double COL_HEADER_HEIGHT = 5;
    static const constexpr double ROW_HEIGHT = 15;
    static const std::string FONT_PLAIN;
    static const std::string EMPTY_STATUS_MESSAGE;
    static const std::string WINDOW_TITLE;


    // set to false to see errors bubble out to console (default true)
    static const bool CATCH_ERRORS = false;

    // graphical interactors in the window
    GWindow* window;
    GTable* table;
    GButton* loadButton;
    GButton* saveButton;
    GButton* clearButton;
    GTextField* formulaField;
    GLabel* statusLabel;

    // spreadsheet model containing table's data of cells
    Spreadsheet* model;
};

#endif // _stanford123gui_h
