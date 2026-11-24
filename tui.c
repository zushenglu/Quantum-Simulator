#include <curses.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "workflow.h"
#include "qubit.h"

#define MAX_QUBITS 1024
#define CELL_WIDTH 9
#define LABEL_WIDTH 7

typedef struct { int wire, depth; } Cursor;

static void status_line(const char *text) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    if (rows < 1) return;
    move(rows - 1, 0);
    clrtoeol();
    if (text) mvaddnstr(rows - 1, 0, text, cols > 0 ? cols - 1 : 0);
    refresh();
}

/* Read one numeric field in curses mode so Esc can cancel immediately. */
static int read_field(const char *prompt, char *input, size_t capacity) {
    size_t length = 0;
    input[0] = '\0';
    curs_set(1);
    for (;;) {
        int row = LINES > 1 ? LINES - 2 : 0;
        move(row, 0);
        clrtoeol();
        mvaddnstr(row, 0, prompt, COLS - 1);
        addnstr(input, COLS - 1);
        refresh();
        int key = getch();
        if (key == 27 || key == ERR || key == '\n' || key == KEY_ENTER) {
            curs_set(0);
            return key == '\n' || key == KEY_ENTER;
        }
        if (key == KEY_BACKSPACE || key == 127 || key == '\b') {
            if (length) input[--length] = '\0';
        } else if (key >= ' ' && key <= '~' && length + 1 < capacity) {
            input[length++] = (char)key;
            input[length] = '\0';
        }
    }
}

static int read_qubits(int *result) {
    char input[32];
    while (read_field("Number of qubits (1-1024, Esc quits): ", input, sizeof(input))) {
        char *end;
        errno = 0;
        long value = strtol(input, &end, 10);
        if (end != input && *end == '\0' && !errno && value >= 1 && value <= MAX_QUBITS) {
            *result = (int)value;
            return 1;
        }
        status_line("Enter a whole number from 1 to 1024.");
    }
    return 0;
}

static int read_angle(float *result) {
    char input[64];
    while (read_field("RZ angle in radians (Esc cancels): ", input, sizeof(input))) {
        char *end;
        errno = 0;
        float value = strtof(input, &end);
        if (end != input && *end == '\0' && !errno && isfinite(value)) {
            *result = value;
            return 1;
        }
        status_line("Enter a finite angle in radians.");
    }
    return 0;
}

static void draw_editor(Circuit *c, Cursor cursor, int top_wire, int left_depth,
                        const char *message) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    erase();
    if (rows < 6 || cols < 24) {
        if (cols > 0) mvaddnstr(0, 0, "Resize terminal to at least 24x6; q quits.", cols - 1);
        refresh();
        return;
    }

    mvaddnstr(0, 0, "Circuit editor | arrows move | a add | e edit | d delete | r run | q quit", cols - 1);
    char selection[120];
    const Operation *current = circuit_gate_at(c, cursor.wire, cursor.depth);
    snprintf(selection, sizeof(selection), "q%d, column %d", cursor.wire, cursor.depth);
    size_t used = strlen(selection);
    if (current && current->impacted_qbts_num == 2) {
        snprintf(selection + used, sizeof(selection) - used, "   CX control q%d -> target q%d",
                 current->impacted_qbts[0], current->impacted_qbts[1]);
    } else if (current && current->parameters) {
        snprintf(selection + used, sizeof(selection) - used, "   %s %.7g rad",
                 current->name, crealf(current->parameters[0]));
    }
    mvaddnstr(1, 0, selection, cols - 1);

    int visible_cols = (cols - LABEL_WIDTH) / CELL_WIDTH;
    int bottom = top_wire + rows - 5;
    if (bottom > c->size) bottom = c->size;
    mvaddstr(3, 0, "wire");
    for (int x = 0; x < visible_cols; x++) {
        mvprintw(3, LABEL_WIDTH + x * CELL_WIDTH + 2, "%d", left_depth + x);
    }
    for (int wire = top_wire; wire < bottom; wire++) {
        int row = 4 + wire - top_wire;
        mvprintw(row, 0, "q%d", wire);
        mvhline(row, LABEL_WIDTH, '-', visible_cols * CELL_WIDTH);
    }
    /* Connect each CX once, including gates with an endpoint offscreen. */
    for (int wire = 0; wire < c->size; wire++) {
        for (const Operation *op = c->Q[wire]->next; op; op = op->next) {
            int x = op->depth - left_depth;
            if (x >= visible_cols) break;
            if (x < 0 || op->impacted_qbts_num != 2 || op->param_ind != 0) continue;
            int other = op->impacted_qbts[1];
            int start = wire < other ? wire : other;
            int end = wire < other ? other : wire;
            for (int w = start < top_wire ? top_wire : start + 1; w < end && w < bottom; w++) {
                mvaddch(4 + w - top_wire, LABEL_WIDTH + x * CELL_WIDTH + 2, '|');
            }
        }
    }
    for (int wire = top_wire; wire < bottom; wire++) {
        int row = 4 + wire - top_wire;
        for (int x = 0; x < visible_cols; x++) {
            int depth = left_depth + x;
            int col = LABEL_WIDTH + x * CELL_WIDTH + 2;
            const Operation *op = circuit_gate_at(c, wire, depth);
            if (op) {
                const char *label = op->name;
                if (op->impacted_qbts_num == 2) label = op->param_ind == 0 ? "o" : "+";
                mvaddnstr(row, col, label, CELL_WIDTH - 2);
            }
            if (wire == cursor.wire && depth == cursor.depth) {
                mvchgat(row, col - 1, 4, A_REVERSE, 0, NULL);
            }
        }
    }
    status_line(message);
}

static const char *gate_name(size_t i) {
    static const char *names[] = { "Hadamard (H)", "Pauli X", "RZ rotation", "Controlled X (CX)" };
    return names[i];
}

static int choose(const char *title, int count, const char *(*name)(size_t), int selected) {
    int first = 0;
    for (;;) {
        erase();
        mvaddnstr(0, 0, title, COLS - 1);
        int capacity = LINES > 2 ? LINES - 2 : 1;
        if (selected < first) first = selected;
        if (selected >= first + capacity) first = selected - capacity + 1;
        for (int i = first; i < count && i < first + capacity; i++) {
            if (i == selected) attron(A_REVERSE);
            mvaddnstr(2 + i - first, 0, name((size_t)i), COLS - 1);
            if (i == selected) attroff(A_REVERSE);
        }
        refresh();
        int key = getch();
        if (key == 27 || key == ERR) return -1;
        if (key == KEY_UP && selected > 0) selected--;
        if (key == KEY_DOWN && selected + 1 < count) selected++;
        if (key == '\n' || key == KEY_ENTER) return selected;
    }
}

static int choose_cx_target(Circuit *c, int control, Cursor cursor, int *target) {
    const char *prompt = "Choose CX target: Up/Down, Enter to place, Esc to cancel.";
    int top = 0;
    for (;;) {
        int height = LINES > 5 ? LINES - 5 : 1;
        if (cursor.wire < top) top = cursor.wire;
        if (cursor.wire >= top + height) top = cursor.wire - height + 1;
        draw_editor(c, cursor, top, cursor.depth, prompt);
        int key = getch();
        if (key == 27 || key == ERR) return 0;
        if (key == KEY_UP && cursor.wire > 0) cursor.wire--;
        if (key == KEY_DOWN && cursor.wire + 1 < c->size) cursor.wire++;
        if (key == '\n' || key == KEY_ENTER) {
            if (cursor.wire != control) {
                *target = cursor.wire;
                return 1;
            }
            prompt = "Target must differ from the control. Use Up/Down or Esc.";
        }
    }
}

static void run_selected_workflow(Circuit *c) {
    int selected = choose("Run workflow (Enter runs, Esc returns)", (int)workflow_count(), workflow_name, 0);
    if (selected < 0) return;
    def_prog_mode();
    endwin();
    int result = workflow_run((size_t)selected, c);
    printf("\nWorkflow %s. Press Enter to return to the editor.\n",
           result == 0 ? "completed" : "failed");
    fflush(stdout);
    (void)getchar();
    reset_prog_mode();
    clear();
    refresh();
}


int main(void) {
    int qubit_count;
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (!read_qubits(&qubit_count)) {
        endwin();
        return 0;
    }
    Circuit *circuit = INIT_CIRCUIT(qubit_count);
    if (!circuit) {
        endwin();
        fprintf(stderr, "Could not allocate circuit.\n");
        return 1;
    }

    Cursor cursor = { 0, 1 };
    int top_wire = 0;
    int left_depth = 1;
    int done = 0;
    char message[256] = "a adds, e edits, d deletes, and r runs a workflow.";

    while (!done) {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        int visible_wires = rows > 5 ? rows - 5 : 1;
        int visible_cols = cols > LABEL_WIDTH ? (cols - LABEL_WIDTH) / CELL_WIDTH : 1;
        if (visible_cols < 1) visible_cols = 1;
        if (cursor.wire < top_wire) top_wire = cursor.wire;
        if (cursor.wire >= top_wire + visible_wires) top_wire = cursor.wire - visible_wires + 1;
        if (cursor.depth < left_depth) left_depth = cursor.depth;
        if (cursor.depth - left_depth >= visible_cols) left_depth = cursor.depth - visible_cols + 1;
        draw_editor(circuit, cursor, top_wire, left_depth, message);

        int key = getch();
        if (key == ERR) break;
        message[0] = '\0';
        if (key == KEY_UP && cursor.wire > 0) cursor.wire--;
        else if (key == KEY_DOWN && cursor.wire + 1 < circuit->size) cursor.wire++;
        else if (key == KEY_LEFT && cursor.depth > 1) cursor.depth--;
        else if (key == KEY_RIGHT && cursor.depth < INT_MAX - 1) cursor.depth++;
        else if (key == 'q' || key == 'Q') done = 1;
        else if (key == 'r' || key == 'R') run_selected_workflow(circuit);
        else if (key == 'd' || key == 'D') {
            const char *error = circuit_remove_gate(circuit, cursor.wire, cursor.depth);
            snprintf(message, sizeof(message), "%s", error ? error : "Gate removed.");
        } else if (key == 'a' || key == 'A' || key == 'e' || key == 'E') {
            int editing = key == 'e' || key == 'E';
            const Operation *existing = circuit_gate_at(circuit, cursor.wire, cursor.depth);
            if (editing && !existing) {
                snprintf(message, sizeof(message), "No gate at the selected cell.");
                continue;
            }
            CircuitGateKind old_kind = CIRCUIT_H;
            if (existing) {
                if (existing->impacted_qbts_num > 1) old_kind = CIRCUIT_CX;
                else if (existing->param_num > 0) old_kind = CIRCUIT_RZ;
                else if (existing->name && strcmp(existing->name, "X") == 0) old_kind = CIRCUIT_X;
            }
            int choice = choose(editing ? "Edit gate (Enter selects, Esc cancels)" :
                                  "Add gate (Enter selects, Esc cancels)", 4, gate_name, old_kind);
            if (choice < 0) {
                snprintf(message, sizeof(message), "Cancelled.");
                continue;
            }

            CircuitGateKind kind = (CircuitGateKind)choice;
            int control = cursor.wire;
            int target = cursor.wire;
            float angle = 0.0f;
            if (kind == CIRCUIT_CX) {
                if (editing && old_kind == CIRCUIT_CX) {
                    control = existing->impacted_qbts[0];
                    target = existing->impacted_qbts[1];
                }
                if (!choose_cx_target(circuit, control, (Cursor){target, cursor.depth}, &target)) {
                    snprintf(message, sizeof(message), "Cancelled.");
                    continue;
                }
            }
            if (kind == CIRCUIT_RZ && !read_angle(&angle)) {
                snprintf(message, sizeof(message), "Cancelled.");
                continue;
            }
            CircuitGateEdit edit = { kind, control, cursor.depth, target, angle };
            const char *error = editing
                ? circuit_update_gate(circuit, cursor.wire, cursor.depth, edit)
                : circuit_place_gate(circuit, edit);
            snprintf(message, sizeof(message), "%s", error ? error :
                     (editing ? "Gate updated." : "Gate placed."));
        }
    }

    endwin();
    FREE_CIRCUIT(circuit);
    return 0;
}
