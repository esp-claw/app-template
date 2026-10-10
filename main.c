/*
 * SPDX-FileCopyrightText: 2026 esp-claw contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
 * myapp: template for an esp-claw WASM app.
 *   myapp_greet  greet `name` 1-5 times (`count`, default 1)
 * The LLM sees each tool as "<manifest name>_<registered name>".
 */
#define CLAW_ARENA_SIZE 1024 /* largest accepted argument JSON */
#include "claw.h"
#include "claw_util.h"

CLAW_DEFINE_ARENA()

static int32_t s_greet = -1;
static char s_out[2048];

CLAW_EXPORT("app_init") void app_init(void)
{
    s_greet = CLAW_REGISTER("greet", "Greet someone by name (template example tool).",
                            "{\"type\":\"object\",\"properties\":{"
                            "\"name\":{\"type\":\"string\",\"description\":\"Who to greet\"},"
                            "\"count\":{\"type\":\"integer\",\"minimum\":1,\"maximum\":5,"
                            "\"description\":\"How many greetings (default 1)\"}},"
                            "\"required\":[\"name\"]}");
}

static void tool_greet(const char *args, const char *end)
{
    claw_buf_t b = { s_out, sizeof(s_out), 0, 0 };
    char name[128];
    /* cj_string keeps JSON escapes as they are, so `name` can be written back into a JSON string. */
    int32_t name_len = cj_string(cj_member(args, end, "name"), end, name, sizeof(name));
    int64_t count = cj_uint(cj_member(args, end, "count"), end);

    if (name_len <= 0) {
        CLAW_RESULT("{\"error\":\"name is required (1-127 bytes)\"}");
        return;
    }
    if (count < 1 || count > 5) {
        count = 1;
    }
    cb_put(&b, "{\"greetings\":[");
    for (int64_t i = 0; i < count; i++) {
        cb_put(&b, i ? ",\"Hello, " : "\"Hello, ");
        cb_putn(&b, name, (uint32_t)name_len);
        cb_put(&b, "!\"");
    }
    cb_put(&b, "]}");
    if (b.full) {
        CLAW_RESULT("{\"error\":\"result too long\"}");
        return;
    }
    CLAW_LOG(CLAW_LOG_INFO, "greet called");
    claw_tool_result(b.buf, b.len);
}

CLAW_EXPORT("tool_call") int32_t tool_call(int32_t tool, const char *args, uint32_t len)
{
    if (tool == s_greet) {
        tool_greet(args, args + len);
        return 0;
    }
    return -1;
}
