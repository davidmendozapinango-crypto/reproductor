#include <assert.h>
#include <stdio.h>

#include "../include/ui.h"

int main(void)
{
    assert(ui_is_predefined_command("next") == 1);
    assert(ui_is_predefined_command("back") == 1);
    assert(ui_is_predefined_command("shuffle") == 1);
    assert(ui_is_predefined_command("loop") == 1);
    assert(ui_is_predefined_command("clear_queue") == 1);
    assert(ui_is_predefined_command("catalog") == 1);
    assert(ui_is_predefined_command("lists") == 1);
    assert(ui_is_predefined_command("load") == 1);
    assert(ui_is_predefined_command("play \"The Search\"") == 1);
    assert(ui_is_predefined_command("queue \"Back in Black\"") == 1);
    assert(ui_is_predefined_command("songs Playlist1") == 1);
    assert(ui_is_predefined_command("new \"QA\": Back in Black") == 1);

    assert(ui_is_predefined_command("") == 0);
    assert(ui_is_predefined_command("   ") == 0);
    assert(ui_is_predefined_command("remove all") == 0);
    assert(ui_is_predefined_command("format disk") == 0);

    printf("Tests passed: UX predefined commands parser.\n");
    return 0;
}
