#include <concord/discord.h>
#include <concord/discord_codecs.h>
#include <concord/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

unsigned long GUILD_ID = 0;
unsigned long g_app_id = 0;

typedef struct QC_Map {
    char* name;
    char* image_url;
} qc_map_t;

qc_map_t qc_maps[] = {
    {
        .name = "Awoken",
        .image_url = "https://static.wikia.nocookie.net/quake/images/8/8c/"
                     "QC_Loading_Screen_Awoken-768x432.png/revision/"
                     "latest?cb=20221219140440",
    },
    // {
    //     .name = "dm6",
    //     .image_url = "https://static.wikia.nocookie.net/quake/images/4/45/"
    //                  "QC_Loading_Screen_Blood_Covenant-768x432.png/revision/"
    //                  "latest?cb=20221025185737",
    // },
    // {
    //     .name = "ztn HAHAHA",
    //     .image_url =
    //         "https://static.wikia.nocookie.net/quake/images/a/ab/"
    //         "Levelshot_blood_run_sm.jpg/revision/latest?cb=20221015225824",
    // },
    {
        .name = "Corrupted Keep",
        .image_url = "https://static.wikia.nocookie.net/quake/images/0/00/"
                     "QC_Loading_Screen_Keep-768x432.png/revision/"
                     "latest?cb=20220929092004",
    },
    {
        .name = "Crucible",
        .image_url = "https://static.wikia.nocookie.net/quake/images/0/05/"
                     "Crucible_ls.png/revision/latest/scale-to-width-down/"
                     "1000?cb=20221217194426",
    },
    {
        .name = "Deep Embrace",
        .image_url =
            "https://static.wikia.nocookie.net/quake/images/3/33/"
            "Deep_embrace_loading_screen.JPG/revision/latest?cb=20230102110619",
    },
    {
        .name = "Insomnia",
        .image_url = "https://static.wikia.nocookie.net/quake/images/3/32/"
                     "Loading-Screen-Insomnia-1024x432.png/revision/latest/"
                     "scale-to-width-down/1000?cb=20221222233305",
    },
    {
        .name = "Ruins of Sarnath",
        .image_url =
            "https://static.wikia.nocookie.net/quake/images/c/c3/Eye.png/"
            "revision/latest/scale-to-width-down/1000?cb=20180621011309",
    },
    {
        .name = "Molten",
        .image_url = "https://static.wikia.nocookie.net/quake/images/3/3b/"
                     "QC_Loading_Screen_Molten_Falls-768x432.png/revision/"
                     "latest?cb=20221123185843",
    },
    {
        .name = "Vale",
        .image_url = "https://static.wikia.nocookie.net/quake/images/c/ce/"
                     "QC_Loading_Screen_Vale_of_Pnath-768x432.png/revision/"
                     "latest?cb=20220929080823",
    },
    {
        .name = "Exile",
        .image_url = "https://static.wikia.nocookie.net/quake/images/c/c7/"
                     "QC_Loading_Screen_Exile-1170x658.png/revision/latest/"
                     "scale-to-width-down/1000?cb=20221013094705",
    },
};
char champs[16][16] = {"Nyx",    "Anarki", "Slash",  "Athena",
                       "Ranger", "DK",     "Doom",   "Eisen",
                       "Galena", "Visor",  "Blazko", "Strogg",
                       "Sorlag", "Keel",   "Clutch", "Scalebearer"};

char weapons[7][18] = {"HMG", "SSG",     "Nailgun", "Rockets",
                       "LG",  "Railgun", "Tribolt"};

int random_number(int min, int max) {
    return random() % (max - min + 1) + min;
}

void fail_interaction_create(struct discord* client,
                             struct discord_response* resp) {
    log_error("%s", discord_strerror(resp->code, client));
}

void on_ready(struct discord* client, const struct discord_ready* event) {
    log_info("packetmanipulator connected to discord server as %s#%s",
             event->user->username, event->user->discriminator);
    g_app_id = event->application->id;
}

void on_slash_command_create(struct discord* client,
                             const struct discord_message* event) {
    log_info("message received. creating commands...");
    if (event->author->bot)
        return;

    struct discord_create_guild_application_command params = {
        .name = "ping", .description = "healthcheck"};

    CCORDcode result = discord_create_guild_application_command(
        client, g_app_id, GUILD_ID, &params, NULL);

    log_info("creating ping command: %d\n", result);

    struct discord_application_command_option options[] = {
        {
            .type = DISCORD_APPLICATION_OPTION_STRING,
            .name = "p1",
            .description = "name of player1.",
            .required = true,
        },
        {
            .type = DISCORD_APPLICATION_OPTION_STRING,
            .name = "p2",
            .description = "name of player2.",
            .required = true,
        },
        {
            .type = DISCORD_APPLICATION_OPTION_BOOLEAN,
            .name = "ban_weapons",
            .description = "if enabled, rolls a banned weapon for each player.",
            .required = false,
        },
    };

    struct discord_create_guild_application_command duel_params = {
        .name = "duel",
        .description = "randomize a duel match.",
        .default_permission = true,
        .options =
            &(struct discord_application_command_options){
                .size = sizeof(options) / sizeof(*options),
                .array = options,
            },
    };

    discord_create_guild_application_command(client, g_app_id, GUILD_ID,
                                             &duel_params, NULL);
    log_info("duel command created.");
}

void on_command_duel(struct discord* client,
                     const struct discord_interaction* event) {
    // return if somehow missing user input
    if (!event->data || !event->data->options)
        return;
    // retrieve information from command
    char* player1 = "";
    char* player2 = "";
    char* test = "";
    int enable_weapon_bans = 0;

    for (int i = 0; i < event->data->options->size; ++i) {
        char* name = event->data->options->array[i].name;
        char* value = event->data->options->array[i].value;

        if (strcmp(name, "p1") == 0) {
            player1 = value;
        } else if (strcmp(name, "p2") == 0) {
            player2 = value;
        } else if (strcmp(name, "ban_weapons") == 0) {
            if (strcmp(value, "true") == 0) {
                enable_weapon_bans = true;
            } else {
                false;
            }
        }
    }

    int map_roll = random_number(0, 10);
    log_info("map_roll %d", map_roll);
    qc_map_t* map = &qc_maps[map_roll];

    int player1_roll = random_number(0, 15);
    log_info("player1_roll %d", player1_roll);
    char* player1_champ = champs[player1_roll];

    int player2_roll = random_number(0, 15);
    log_info("player2_roll %d", player2_roll);
    char* player2_champ = champs[player2_roll];

    char* player1_weapon = NULL;
    char* player2_weapon = NULL;
    char banned_weapons[32] = "";

    log_info("weapon_bans: %d", enable_weapon_bans);
    if (enable_weapon_bans) {
        int weapon1_roll = random_number(0, 6);
        log_info("weapon1_roll: %d", weapon1_roll);
        int weapon2_roll = random_number(0, 6);
        log_info("weapon2_roll: %d", weapon2_roll);
        snprintf(banned_weapons, sizeof(banned_weapons), "%s\t | \t%s",
                 weapons[weapon1_roll], weapons[weapon2_roll]);
    } else {
        snprintf(banned_weapons, sizeof(banned_weapons), "none");
    }

    char title[64] = "";
    snprintf(title, sizeof(title), "%s\t vs \t%s", player1, player2);

    char champions[64] = "";
    snprintf(champions, sizeof(champions), "%s\t | \t%s", player1_champ,
             player2_champ);

    struct discord_embed_field fields[] = {
        {.name = "champs", .value = champions},
        {.name = "banned weapons", .value = banned_weapons},
        {.name = map->name, .value = ""}};

    struct discord_embed embeds[] = {
        {.title = title,
         .color = 0x3498db,
         .timestamp = discord_timestamp(client),
         // .image = &(struct discord_embed_image){.url = map->image_url},
         .thumbnail = &(struct discord_embed_thumbnail){.url = map->image_url},
         .fields = &(struct discord_embed_fields){
             .size = sizeof(fields) / sizeof(*fields), .array = fields}}};

    struct discord_interaction_response duel_params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data){
            .embeds = &(struct discord_embeds){
                .size = sizeof(embeds) / sizeof(*embeds),
                .array = embeds,
            }}};

    discord_create_interaction_response(client, event->id, event->token,
                                        &duel_params, NULL);
}

void on_interaction_create(struct discord* client,
                           const struct discord_interaction* event) {
    // only handle slash commands
    if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND)
        return;

    log_info("event happened!");
    // return if somehow missing user input
    // if (!event->data || !event->data->options)
    //     return;

    log_info(
        "event received:\n"
        "\t name: %s",
        event->data->name);

    if (strcmp(event->data->name, "duel") == 0) {
        on_command_duel(client, event);
        // char* player1 = "";
        // char* player2 = "";
        // int enable_weapon_bans = 0;
        //
        // for (int i = 0; i < event->data->options->size; ++i) {
        //     char* name = event->data->options->array[i].name;
        //     char* value = event->data->options->array[i].value;
        //
        //     if (strcmp(name, "p1") == 0) {
        //         player1 = value;
        //     } else if (strcmp(name, "p2") == 0) {
        //         player2 = value;
        //     } else if (strcmp(name, "ban_weapons") == 0) {
        //         enable_weapon_bans = strtol(value, NULL, 10);
        //     }
        // }
        //
        // char buf[DISCORD_MAX_MESSAGE_LEN] = "";
        // snprintf(buf, sizeof(buf),
        //          "%s\t vs \t%s\n"
        //          "weapon bans: %d"
        //          "map: Awoken\n"
        //          "Visor\t vs \t Athena\n",
        //          player1, player2, enable_weapon_bans);
        //
        // struct discord_interaction_response duel_params = {
        //     .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        //     .data =
        //         &(struct discord_interaction_callback_data){.content = buf}};
        //
        // struct discord_ret_interaction_response ret = {
        //     .fail = &fail_interaction_create};
        //
        // discord_create_interaction_response(client, event->id, event->token,
        //                                     &duel_params, &ret);
    }

    if (strcmp(event->data->name, "ping") == 0) {
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data =
                &(struct discord_interaction_callback_data){.content = "pong"}};

        struct discord_ret_interaction_response ret = {
            .fail = &fail_interaction_create};

        discord_create_interaction_response(client, event->id, event->token,
                                            &params, &ret);
    }
}

int main(void) {
    srandom(time(NULL));
    struct ccord_szbuf_readonly value;

    struct discord* client = discord_config_init("../config.json");

    // get the guild id from config file
    value =
        discord_config_get_field(client, (char*[]){"custom", "guild-id"}, 2);
    GUILD_ID = strtol(value.start, NULL, 10);

    discord_set_on_ready(client, &on_ready);
    discord_set_on_command(client, "!commands_create",
                           &on_slash_command_create);
    discord_set_on_interaction_create(client, &on_interaction_create);
    discord_run(client);

    discord_cleanup(client);
    ccord_global_cleanup();
}
