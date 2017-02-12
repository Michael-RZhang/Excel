/**
 * CS 106B/X Stanford 1-2-3
 * This file implements the ssview.h interface, which provides
 * the graphical view for the Stanford 1-2-3 program.
 * This file also contains the overall main() function to run the program.
 *
 * Please do not modify this provided file. Your turned-in files should work
 * with an unmodified version of all provided code files.
 *
 * @author Marty Stepp, Julie Zelenski, Jerry Cain
 * @version 2016/11/29
 * - 16au 106x version (stepp)
 */

#include "stanford123gui.h"
#include "filelib.h"
#include "gevents.h"
#include "gfilechooser.h"
#include "goptionpane.h"
#include "private/platform.h"

const std::string Stanford123Gui::WINDOW_TITLE = "Stanford 1-2-3";
const std::string Stanford123Gui::FONT_PLAIN = "SansSerif-Plain-12";
const std::string Stanford123Gui::EMPTY_STATUS_MESSAGE = "<html>&nbsp;</html>";

Stanford123Gui::Stanford123Gui(Spreadsheet* model) {
    this->model = model;

    // create window and controls
    window = new GWindow((COLS_TO_DISPLAY_COUNT + 1) * COL_WIDTH,
                          COL_HEADER_HEIGHT + (ROWS_TO_DISPLAY_COUNT + 2) * ROW_HEIGHT + 1,
                         /* visible */ false);
    window->setCloseOperation(GWindow::CLOSE_DO_NOTHING);
    window->setTitle(WINDOW_TITLE + " - Untitled");
    window->setResizable(true);

    loadButton = new GButton("Load");
    loadButton->setIcon("icon_load.gif");
    saveButton = new GButton("Save");
    saveButton->setIcon("icon_save.gif");
    clearButton = new GButton("Clear");
    clearButton->setIcon("icon_clear.gif");

    formulaField = new GTextField(40);
    formulaField->setPlaceholder("cell value/formula editor (or double-click / press F2 on a cell)");
    formulaField->addActionListener();

    statusLabel = new GLabel(EMPTY_STATUS_MESSAGE);

    table = new GTable(ROWS_TO_DISPLAY_COUNT, COLS_TO_DISPLAY_COUNT + 1);
    table->setEditable(true);
    table->setColumnHeaderStyle(GTable::COLUMN_HEADER_EXCEL);
    table->setRowColumnHeadersVisible(true);
    table->setFont(FONT_PLAIN);
    table->setHorizontalAlignment(GTable::Alignment::LEFT);

    // do layout
    window->addToRegion(table, GWindow::REGION_CENTER);
    window->setRegionAlignment(GWindow::REGION_NORTH, GWindow::ALIGN_LEFT);
    window->addToRegion(loadButton, GWindow::REGION_NORTH);
    window->addToRegion(saveButton, GWindow::REGION_NORTH);
    window->addToRegion(clearButton, GWindow::REGION_NORTH);
    window->addToRegion(formulaField, GWindow::REGION_NORTH);
    window->addToRegion(statusLabel, GWindow::REGION_SOUTH);

    window->center();
    window->setVisible(true);
    table->requestFocus();
    table->select(0, 0);
}

void Stanford123Gui::clear() {
    if (model) {
        model->clear();
    }
    setDocumentModified();
    table->requestFocus();
}

void Stanford123Gui::clearCells() {
    table->clear();
    clearStatusMessage();
    updateFormulaFieldText();
    table->requestFocus();
}

void Stanford123Gui::clearStatusMessage() {
    setStatusMessage(/* message */ EMPTY_STATUS_MESSAGE, /* isError */ false);
}

void Stanford123Gui::displayCell(int row, int column, const std::string& text) {
    if (table->inBounds(row, column)) {
        table->set(row, column, text);
        updateFormulaFieldText();
    } else {
        error("displayCell: cell out of range: R" + integerToString(row)
              + "C" + integerToString(column));
    }
}

void Stanford123Gui::displayCell(const std::string& cellname, const std::string& text) {
    int row, col;
    if (Range::toRowColumn(cellname, row, col)) {
        displayCell(row, col, text);
    } else {
        error("displayCell: invalid cell name: \"" + cellname + "\"");
    }
}

void Stanford123Gui::eventLoop() {
    while (true) {
        GEvent event = waitForEvent(ACTION_EVENT | KEY_EVENT | TABLE_EVENT | WINDOW_EVENT);
        if (event.getEventClass() == ACTION_EVENT) {
            GActionEvent actionEvent(event);
            processActionEvent(actionEvent);
        } else if (event.getEventClass() == KEY_EVENT) {
            GKeyEvent keyEvent(event);
            processKeyEvent(keyEvent);
        } else if (event.getEventClass() == TABLE_EVENT) {
            GTableEvent tableEvent(event);
            processTableEvent(tableEvent);
        } else if (event.getEventClass() == WINDOW_EVENT) {
            GWindowEvent windowEvent(event);
            if (!processWindowEvent(windowEvent)) {
                break;
            }
        }
    }
}

bool Stanford123Gui::isDocumentModified() const {
    std::string title = window->getTitle();
    return endsWith(title, " *");
}

void Stanford123Gui::load() {
    std::string filename = GFileChooser::showOpenDialog("", "*.123");
    if (filename.empty() || !fileExists(filename)) {
        return;
    }

    if (isDocumentModified() && GOptionPane::showConfirmDialog(
                "You have unsaved changes. Really load?", "", GOptionPane::YES_NO)
            != GOptionPane::YES) {
        return;
    }

    std::ifstream infile;
    infile.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
    if (!infile.fail()) {
        table->clear();
        model->load(infile);
        infile.close();
        setStatusMessage("Data loaded from " + getTail(filename) + ".");
        window->setTitle(WINDOW_TITLE + " - " + getTail(filename));
        updateFormulaFieldText();
        table->requestFocus();
    }
}

void Stanford123Gui::processActionEvent(GActionEvent& actionEvent) {
    GObject* src = actionEvent.getSource();
    if (src == loadButton) {
        load();
    } else if (src == saveButton) {
        save();
    } else if (src == clearButton) {
        clear();
    } else if (src == formulaField) {
        int row = table->getSelectedRow();
        int col = table->getSelectedColumn();
        if (table->inBounds(row, col)) {
            std::string exprText = formulaField->getText();
            std::string cellname = Range::toCellName(row, col);

            if (CATCH_ERRORS) {
                try {
                    model->setCell(cellname, exprText);
                    clearStatusMessage();
                    setDocumentModified();
                } catch (ErrorException& ex) {
                    setStatusMessage(ex.getMessage(), /* isError */ true);
                }
            } else {
                // run without catching errors/exceptions
                model->setCell(cellname, exprText);
                clearStatusMessage();
                setDocumentModified();
            }
        }
    }
}

void Stanford123Gui::processKeyEvent(GKeyEvent& keyEvent) {
    if (keyEvent.getEventType() == KEY_PRESSED) {
        int code = keyEvent.getKeyCode();
        bool ctrl = keyEvent.isCtrlOrCommandKeyDown();
        if (ctrl && code == DELETE_KEY) {
            clear();
        } else if (ctrl && (code == 'L' || code == 'O')) {
            load();
        } else if (ctrl && code == 'S') {
            save();
        }
    }
}

void Stanford123Gui::processTableEvent(GTableEvent& tableEvent) {
    int row = tableEvent.getRow();
    int col = tableEvent.getColumn();
    std::string cellname = Range::toCellName(row, col);

    if (tableEvent.getEventType() == TABLE_CUT || tableEvent.getEventType() == TABLE_COPY) {
        // actually copy the cell's formula, not its displayed value
        std::string formula = model->getCellRawText(cellname);
        if (!formula.empty()) {
            stanfordcpplib::getPlatform()->clipboard_set(formula);
        }
    } else if (tableEvent.getEventType() == TABLE_EDIT_BEGIN) {
        // actually edit the cell's formula, not its displayed value
        if (model->cellIsFormula(cellname)) {
            std::string cellText = model->getCellRawText(cellname);
            if (!cellText.empty()) {
                setCellEditorValue(cellname, cellText);
            }
        }
    } else if (tableEvent.getEventType() == TABLE_PASTE) {
        // empty
    } else if (tableEvent.getEventType() == TABLE_SELECTED) {
        updateFormulaFieldText();
    } else if (tableEvent.getEventType() == TABLE_UPDATED) {
        table->set(row, col, "");   // temporary; spreadsheet must fix

        std::string oldValue = model->getCellRawText(cellname);
        std::string value = tableEvent.getValue();

        if (CATCH_ERRORS) {
            try {
                model->setCell(cellname, value);
                clearStatusMessage();
                setDocumentModified();
                updateFormulaFieldText();
            } catch (const ErrorException& ex) {
                setStatusMessage(ex.getMessage(), /* isError */ true);
                std::cout << ex << std::endl;
                table->set(row, col, oldValue);
            }
        } else {
            // run without catching errors/exceptions
            model->setCell(cellname, value);
            clearStatusMessage();
            setDocumentModified();
            updateFormulaFieldText();
        }
    }
}

bool Stanford123Gui::processWindowEvent(GWindowEvent& windowEvent) {
    if (windowEvent.getEventType() == WINDOW_CLOSING) {
        if (isDocumentModified() && GOptionPane::showConfirmDialog(
                    "You have unsaved changes. Really exit?", "", GOptionPane::YES_NO)
                != GOptionPane::YES) {
            return true;
        }

        window->setCloseOperation(GWindow::CLOSE_HIDE);
        window->close();
        return false;   // stop event loop
    }
    return true;
}

void Stanford123Gui::save() {
    std::string filename = GFileChooser::showSaveDialog("", "*.123");
    if (filename.empty()) {
        return;
    }
    if (fileExists(filename)) {
        if (GOptionPane::showConfirmDialog("File exists. Overwrite?") != GOptionPane::YES) {
            return;
        }
    }

    std::ofstream outfile;
    outfile.open(filename.c_str(), std::ios_base::binary | std::ios_base::out);
    if (!outfile.fail()) {
        model->save(outfile);
        outfile.close();
        setStatusMessage("Data saved to " + getTail(filename) + ".");
        window->setTitle(WINDOW_TITLE + " - " + getTail(filename));
        table->requestFocus();
    }
}

void Stanford123Gui::setCellEditorValue(const std::string& cellname, const std::string& txt) {
    int row, col;
    if (Range::toRowColumn(cellname, row, col)) {
        table->setEditorValue(row, col, txt);
    } else {
        error("setCellEditorValue: invalid cell name: \"" + cellname + "\"");
    }
}

void Stanford123Gui::setDocumentModified(bool modified) {
    std::string title = window->getTitle();
    if (modified && !isDocumentModified()) {
        title += " *";
        window->setTitle(title);
    } else if (!modified && isDocumentModified()) {
        title = title.substr(0, title.length() - 2);
        window->setTitle(title);
    }
}

void Stanford123Gui::setModel(Spreadsheet* model) {
    this->model = model;
}

void Stanford123Gui::setStatusMessage(const std::string& message, bool isError) {
    static int STATUS_COLOR = 0x0;        // black
    static int ERROR_COLOR  = 0xbb0000;   // red
    statusLabel->setText(message);
    statusLabel->setColor(isError ? ERROR_COLOR : STATUS_COLOR);
}

void Stanford123Gui::updateFormulaFieldText() {
    int row = table->getSelectedRow();
    int col = table->getSelectedColumn();
    std::string text = "";
    if (table->inBounds(row, col)) {
        std::string cellname = Range::toCellName(row, col);
        text = model->getCellRawText(cellname);
    }
    formulaField->setText(text);
}

int main() {
    std::cout << "Welcome to Stanford 1-2-3!" << std::endl;
    std::cout.flush();

    // create GUI and let it respond to events
    Stanford123Gui* view = new Stanford123Gui();
    Spreadsheet* model = new Spreadsheet((View*) view);
    view->setModel(model);
    view->eventLoop();

    std::cout << "Exiting." << std::endl;
    delete view;
    exitGraphics();
    return 0;
}
