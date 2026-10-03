/* JAVA-EXT-003: does CNA's .cnb model path survive a build/encode/decode/destroy cycle?
   JAVA-UPSTREAM-004 found cna_content_manager_load_model segfaulting during teardown for any
   asset with a mesh part, so the .cnb model family is probed in C before any of it is bound. */
#include <CNA/C/cnb.h>
#include <CNA/C/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static CNA_StringView view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

#define STEP(expression)                                                       \
    do {                                                                       \
        const CNA_Result step_result = (expression);                           \
        printf("%-52s -> %d\n", #expression, (int)step_result);                \
        fflush(stdout);                                                        \
        if (step_result != CNA_RESULT_SUCCESS) {                               \
            return 1;                                                          \
        }                                                                      \
    } while (0)

int main(void)
{
    CNA_CnbModelDataHandle model = 0;
    uint64_t index = 0;
    float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    uint8_t* image = NULL;
    uint64_t size = 0;

    STEP(cna_cnb_model_create(&model));
    STEP(cna_cnb_model_add_bone(model, view("root"), -1, identity, &index));
    printf("bone index %llu\n", (unsigned long long)index);
    STEP(cna_cnb_model_add_bone(model, view("spine"), 0, identity, &index));

    {
        /* One mesh with one part: exactly the shape that killed the content-manager loader. */
        uint64_t mesh = 0;
        uint64_t part = 0;
        uint32_t parts[1];
        const float vertices[9] = {0, 0, 0, 1, 0, 0, 0, 1, 0};
        const uint16_t indices[3] = {0, 1, 2};
        CNA_CnbModelPartInfo info;
        memset(&info, 0, sizeof info);
        info.struct_size = (uint32_t)(sizeof info);
        info.struct_version = UINT32_C(1);
        info.vertex_stride = UINT32_C(12);
        info.vertex_count = UINT32_C(3);
        info.index_count = UINT32_C(3);
        info.index_element_size = UINT32_C(2);
        info.primitive_topology = UINT32_C(4);
        info.primitive_count = UINT32_C(1);
        STEP(cna_cnb_model_add_part(model, &info, view("part0"), view(""), &part));
        printf("part index %llu\n", (unsigned long long)part);
        STEP(cna_cnb_model_set_part_vertex_bytes(model, part, (const uint8_t*)vertices,
                                                 (uint64_t)sizeof vertices));
        STEP(cna_cnb_model_set_part_index_bytes(model, part, (const uint8_t*)indices,
                                                (uint64_t)sizeof indices));
        parts[0] = (uint32_t)part;
        STEP(cna_cnb_model_add_mesh(model, view("mesh0"), 0, parts, UINT64_C(1), &mesh));
        printf("mesh index %llu\n", (unsigned long long)mesh);
    }

    {
        const CNA_Result probe = cna_cnb_encode_model(model, view("models/probe"), NULL,
                                                      UINT64_C(0), &size);
        printf("encode size probe -> %d, %llu bytes\n", (int)probe,
               (unsigned long long)size);
        if (probe != CNA_RESULT_BUFFER_TOO_SMALL && probe != CNA_RESULT_SUCCESS) {
            return 1;
        }
    }
    image = (uint8_t*)malloc((size_t)size + 1U);
    if (image == NULL) {
        return 1;
    }
    STEP(cna_cnb_encode_model(model, view("models/probe"), image, size, &size));
    STEP(cna_cnb_model_destroy(model));

    {
        CNA_CnbDocumentHandle document = 0;
        CNA_CnbModelDataHandle decoded = 0;
        CNA_CnbReadLimits limits;
        memset(&limits, 0, sizeof limits);
        limits.struct_size = (uint32_t)(sizeof limits);
        limits.struct_version = UINT32_C(1);
        STEP(cna_cnb_read_limits_init(&limits));
        STEP(cna_cnb_document_parse(image, size, view("probe.cnb"), &limits, &document));
        STEP(cna_cnb_decode_model(document, &decoded));
        {
            CNA_CnbModelInfo info;
            memset(&info, 0, sizeof info);
            info.struct_size = (uint32_t)(sizeof info);
            info.struct_version = UINT32_C(1);
            STEP(cna_cnb_model_get_info(decoded, &info));
            printf("decoded bones %llu meshes %llu parts %llu\n",
                   (unsigned long long)info.bone_count,
                   (unsigned long long)info.mesh_count,
                   (unsigned long long)info.part_count);
        }
        STEP(cna_cnb_model_destroy(decoded));
        STEP(cna_cnb_document_destroy(document));
    }
    free(image);
    printf("PROBE OK\n");
    return 0;
}
