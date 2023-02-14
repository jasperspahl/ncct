#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ncurses.h>
#include <wait.h>
#include <unistd.h>

#define MAX_COLS 200
#define MAX_LINES 10000

struct Model {
    WINDOW * pad;
    WINDOW * statusbar;
    int cur_y, cur_x, pos_y, pos_x;
    char * string;
};

void read_file(struct Model *model);
void save_file(struct Model *model);
/**
 * Opens an editor
 * @param model State of the application
 * @param mode The char to which opens the edit mode (`o O i I a A`)
 */
void open_editor(struct Model *model, char mode);
void draw(struct Model *model);


static const char * filename = "README.md";

int main(void) {
    printf("\033[2 q");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);

    struct Model model = {0};
    model.pad = newpad(MAX_LINES, MAX_COLS);
    model.statusbar = newwin(1, COLS, LINES-1, 0);
    curs_set(1);

    read_file(&model);

    refresh();
    draw(&model);
    int ch;
    while ((ch = getch()) != 'q')
    {
        switch (ch) {
            case 'j':
            case KEY_DOWN:
                // move down
                if (model.cur_y < MAX_LINES) {
                    model.cur_y++;
                    if (model.cur_y > model.pos_y + LINES-1) {
                        model.pos_y++;
                    }
                }
                break;
            case 'k':
            case KEY_UP:
                // move up
                if (model.cur_y > 0) {
                    model.cur_y--;
                    if (model.cur_y < model.pos_y) {
                        model.pos_y--;
                    }
                }
                break;
            case 'h':
            case KEY_LEFT:
                // move left
                if (model.cur_x > 0) {
                    model.cur_x--;
                    if (model.cur_x < model.pos_x) {
                        model.pos_x--;
                    }
                }
                break;
            case 'l':
            case KEY_RIGHT:
                // move right
                if (model.cur_x < MAX_COLS) {
                    model.cur_x++;
                    if (model.cur_x > model.pos_x + COLS-1) {
                        model.pos_x++;
                    }
                }
                break;
            case 'i':
            case 'I':
            case 'a':
            case 'A':
            case 'o':
            case 'O':
                save_file(&model);
                open_editor(&model, (char)ch);
                read_file(&model);
                break;
            default:
                continue;
        }
        draw(&model);
    }

    endwin();
    return 0;
}

void read_file(struct Model *model) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        endwin();
        fprintf(stderr, "can not open file %s @ %s:%d: %s", filename,__FILE__, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fseek(f, 0, SEEK_END);
    size_t length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char * buffer = malloc(length + 1);
    if (buffer)
    {
        fread(buffer, sizeof(char), length, f);
    }
    buffer[length] = '\0';
    fclose(f);
    if (strlen(buffer) > 0) {
        if (model->string) free(model->string);
        model->string = buffer;
        touchwin(model->pad);
        wclear(model->pad);
        mvwaddstr(model->pad, 0,0, model->string);
    }
}

void save_file(struct Model *model) {
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        endwin();
        fprintf(stderr, "can not open file %s @ %s:%d: %s", filename,__FILE__, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fwrite(model->string, sizeof(char), strlen(model->string), f);
    fclose(f);
}


void open_editor(struct Model *model, char mode)
{
    char * command = malloc(strlen("vim \"+normal 12345G123|i\" +star ") + strlen(filename) + 1);
    sprintf(command,
    "vim \"+normal %dG%d|%c\" +star %s",
            model->cur_y+1,
            model->cur_x+1,
            mode,
            filename);

    if (0 == fork()) {
        execlp("sh", "sh", "-c", command, NULL);
        exit(0);
    }
    free(command);
    wait(NULL);
}

void draw(struct Model *model) {

    pnoutrefresh(model->pad, // source pad
                 model->pos_y, // source pad y
                 model->pos_x, // source pad x
                 0, // dest window y
                 0, // dest window x
                 LINES-2, // dest window height
                 COLS-1 // dest window width
    );
    wattron(model->statusbar, A_REVERSE);
    mvwhline(model->statusbar, 0,0, ' ', getmaxx(model->statusbar));
    mvwprintw(model->statusbar, 0, 1, "Cursor: %d:%d | Position: %d:%d | q: quit", model->cur_y, model->cur_x, model->pos_y, model->pos_x);
    wattroff(model->statusbar, A_REVERSE);
    wnoutrefresh(model->statusbar);
    doupdate();

    move(model->cur_y - model->pos_y, model->cur_x - model->pos_x);
}
