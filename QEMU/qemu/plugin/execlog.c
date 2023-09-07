/*
 * Copyright (C) 2021, Alexandre Iooss <erdnaxe@crans.org>
 *
 * Log instruction execution with memory access.
 *
 * License: GNU GPL, version 2 or later.
 *   See the COPYING file in the top-level directory.
 */
#include <glib.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <qemu-plugin.h>

// #include <bswap.h>

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

//xwt
// extern int global_host_addr_xwt_num;

/* Store last executed instruction on each vCPU as a GString */
static GPtrArray *last_exec;
static GMutex expand_array_lock;

static GPtrArray *imatches;
static GArray *amatches;

long long memory_data_inside;

typedef struct xwt_insn{
    int is_store_load;
    struct qemu_plugin_insn * insn;
}xwt_insn;


/*
 * Expand last_exec array.
 *
 * As we could have multiple threads trying to do this we need to
 * serialise the expansion under a lock. Threads accessing already
 * created entries can continue without issue even if the ptr array
 * gets reallocated during resize.
 */
static void expand_last_exec(int cpu_index)
{
    g_mutex_lock(&expand_array_lock);
    while (cpu_index >= last_exec->len) {
        GString *s = g_string_new(NULL);
        g_ptr_array_add(last_exec, s);
    }
    g_mutex_unlock(&expand_array_lock);
}

static inline uint64_t ldq_he_p(const void *ptr)
{
    uint64_t r;
    __builtin_memcpy(&r, ptr, sizeof(r));
    return r;
}

static inline uint32_t ldl_he_p(const void *ptr)
{
    uint32_t r;
    __builtin_memcpy(&r, ptr, sizeof(r));
    return r;
}


// void *qemu_get_cpu(int index);

// static uint64_t get_cpu_register(unsigned int cpu_index, unsigned int reg) {
//     uint8_t* cpu = qemu_get_cpu(cpu_index);

//     return *(uint64_t *)(cpu + 33488 + 5424 + reg * 8);
// }

/**
 * Add memory read or write information to current instruction log
 */
static void vcpu_mem(unsigned int cpu_index, qemu_plugin_meminfo_t info,
                     uint64_t vaddr, void *udata)
{
    GString *s;

    /* Find vCPU in array */
    g_assert(cpu_index < last_exec->len);
    s = g_ptr_array_index(last_exec, cpu_index);

    /* Indicate type of memory access */
    if (qemu_plugin_mem_is_store(info)) {
        // g_string_append(s, ", store");
        printf("本指令为store类型:\n");
        *((int *)udata) = 1;
    } else {
        // g_string_append(s, ", load");
        printf("本指令为load类型:\n");
        *((int *)udata) = 2;
    }

    /* If full system emulation log physical address and device name */
    struct qemu_plugin_hwaddr *hwaddr = qemu_plugin_get_hwaddr(info, vaddr);
    if (hwaddr) {
        uint64_t addr = qemu_plugin_hwaddr_phys_addr(hwaddr);
        const char *name = qemu_plugin_hwaddr_device_name(hwaddr);
        // g_string_append_printf(s, ", 0x%08"PRIx64", %s", addr, name);
        // printf("addr:%lld name:%s\n",(long long)addr,name);
    } else {
        // g_string_append_printf(s, ", 0x%08"PRIx64, vaddr);
        // printf("vaddr: %lld\n",(long long)vaddr);

        // char *val = (char*) vaddr;
        // printf("vaddr value:%s\n",val);
        
    }
}

/**
 * Log instruction execution
 */
static void vcpu_insn_exec(unsigned int cpu_index, void *udata)
{
    GString *s;

    /* Find or create vCPU in array */
    if (cpu_index >= last_exec->len) {
        expand_last_exec(cpu_index);
    }
    s = g_ptr_array_index(last_exec, cpu_index);

    /* Print previous instruction in cache */
    if (s->len) {
        // qemu_plugin_outs(s->str);
        // qemu_plugin_outs("\n");
    }
    // printf("123xwt\n");
    // xwt_insn *execd_insn = (xwt_insn *)udata;
    // struct qemu_plugin_insn *insn = execd_insn->insn;
    // char *insn_disas = qemu_plugin_insn_disas(insn);
    // uint64_t insn_vaddr = qemu_plugin_insn_vaddr(insn);
    // uint64_t insn_opcode;
    // long long insn_siz = (long long)qemu_plugin_insn_size(insn);
    // long unsigned int insn_machine_code = ldl_he_p(insn_vaddr);
    // insn_opcode = *((uint64_t *)qemu_plugin_insn_data(insn));
    // insn_disas = qemu_plugin_insn_disas(insn);
    // insn_vaddr = qemu_plugin_insn_vaddr(insn);
    // printf("insn_vaddr虚拟地址: 0x%llx\n", (unsigned long long)insn_vaddr);
    // printf("opcode操作码: 0x%llx\n", (unsigned long long)insn_opcode);
    // printf("当前的指令的长度为:%lld字节\n",insn_siz);
    // printf("当前的指令机器码为:%lx\n",insn_machine_code);
    // printf("反汇编码:%s\n",insn_disas);

    // uint64_t value = ldq_he_p(vaddr);
    // printf("xwt vaddr value:%llu\n",value);

    /* Store new instruction in cache */
    /* vcpu_mem will add memory access information to last_exec */
    // g_string_printf(s, "%u, ", cpu_index);
    // g_string_append(s, (char *)udata);
}


/**
 * On translation block new translation
 *
 * QEMU convert code by translation block (TB). By hooking here we can then hook
 * a callback on each instruction and memory access.
 */
int tb_nums=0;
static void vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb *tb)
{
    struct qemu_plugin_insn *insn;
    bool skip = (imatches || amatches);

    //xwt
    printf("tb编号: %d\n",tb_nums);
    tb_nums++;
    size_t n = qemu_plugin_tb_n_insns(tb);
    for (size_t i = 0; i < n; i++) {
        insn = qemu_plugin_tb_get_insn(tb, i);
        char *insn_disas = qemu_plugin_insn_disas(insn);
        uint64_t insn_vaddr = qemu_plugin_insn_vaddr(insn);
        uint64_t insn_opcode;
        long long insn_siz = (long long)qemu_plugin_insn_size(insn);
        long unsigned int insn_machine_code = ldl_he_p(insn_vaddr);
        insn_opcode = *((uint64_t *)qemu_plugin_insn_data(insn));
        insn_disas = qemu_plugin_insn_disas(insn);
        insn_vaddr = qemu_plugin_insn_vaddr(insn);
        printf("insn_vaddr虚拟地址: 0x%llx\n", (unsigned long long)insn_vaddr);
        printf("opcode操作码: 0x%llx\n", (unsigned long long)insn_opcode);
        printf("当前的指令的长度为:%lld字节\n",insn_siz);
        printf("当前的指令机器码为:%lx\n",insn_machine_code);
        printf("反汇编码:%s\n",insn_disas);

        // char *insn_disas;
        // uint64_t insn_vaddr;

        /*
         * `insn` is shared between translations in QEMU, copy needed data here.
         * `output` is never freed as it might be used multiple times during
         * the emulation lifetime.
         * We only consider the first 32 bits of the instruction, this may be
         * a limitation for CISC architectures.
         */

        // insn_disas = qemu_plugin_insn_disas(insn);
        // insn_vaddr = qemu_plugin_insn_vaddr(insn);

        
        /*
         * If we are filtering we better check out if we have any
         * hits. The skip "latches" so we can track memory accesses
         * after the instruction we care about.
         */
        if (skip && imatches) {
            int j;
            for (j = 0; j < imatches->len && skip; j++) {
                char *m = g_ptr_array_index(imatches, j);
                if (g_str_has_prefix(insn_disas, m)) {
                    skip = false;
                }
            }
        }

        if (skip && amatches) {
            int j;
            for (j = 0; j < amatches->len && skip; j++) {
                uint64_t v = g_array_index(amatches, uint64_t, j);
                if (v == insn_vaddr) {
                    skip = false;
                }
            }
        }

        if (skip) {
            g_free(insn_disas);
        } else {
            int is_store_load = 0;
            // 0 : Neither store or load
            // 1 : store
            // 2 : load
            uint64_t insn_opcode;
            insn_opcode = *((uint64_t *)qemu_plugin_insn_data(insn));
            // char *output = g_strdup_printf("0x%"PRIx64", 0x%"PRIx64", \"%s\"",
            //                                insn_vaddr, insn_opcode, insn_disas);
            long long insn_siz = (long long)qemu_plugin_insn_size(insn);
            long unsigned int insn_machine_code = ldl_he_p(insn_vaddr);
            //xwt
            // printf("insn_vaddr虚拟地址:%llu\n",(long unsigned int)insn_vaddr);
            // printf("opcode操作码:%llu\n",(long unsigned int)insn_opcode);

            /* Register callback on memory read or write */
            qemu_plugin_register_vcpu_mem_cb(insn, vcpu_mem,
                                             QEMU_PLUGIN_CB_NO_REGS,
                                             QEMU_PLUGIN_MEM_RW, &is_store_load);

            xwt_insn *last_insn = (xwt_insn *)malloc(sizeof(xwt_insn));
            last_insn->is_store_load = is_store_load;
            last_insn->insn = insn;
            /* Register callback on instruction */
            qemu_plugin_register_vcpu_insn_exec_cb(insn, vcpu_insn_exec,
                                                   QEMU_PLUGIN_CB_NO_REGS, last_insn);

            /* reset skip */
            skip = (imatches || amatches);
        }

    }
}

/**
 * On plugin exit, print last instruction in cache
 */
static void plugin_exit(qemu_plugin_id_t id, void *p)
{
    guint i;
    GString *s;
    for (i = 0; i < last_exec->len; i++) {
        s = g_ptr_array_index(last_exec, i);
        if (s->str) {
            qemu_plugin_outs(s->str);
            qemu_plugin_outs("\n");
        }
    }
}

/* Add a match to the array of matches */
static void parse_insn_match(char *match)
{
    if (!imatches) {
        imatches = g_ptr_array_new();
    }
    g_ptr_array_add(imatches, match);
}

static void parse_vaddr_match(char *match)
{
    uint64_t v = g_ascii_strtoull(match, NULL, 16);

    if (!amatches) {
        amatches = g_array_new(false, true, sizeof(uint64_t));
    }
    g_array_append_val(amatches, v);
}

/**
 * Install the plugin
 */
QEMU_PLUGIN_EXPORT int qemu_plugin_install(qemu_plugin_id_t id,
                                           const qemu_info_t *info, int argc,
                                           char **argv)
{
    /*
     * Initialize dynamic array to cache vCPU instruction. In user mode
     * we don't know the size before emulation.
     */
    if (info->system_emulation) {
        last_exec = g_ptr_array_sized_new(info->system.max_vcpus);
    } else {
        last_exec = g_ptr_array_new();
    }

    for (int i = 0; i < argc; i++) {
        char *opt = argv[i];
        g_autofree char **tokens = g_strsplit(opt, "=", 2);
        if (g_strcmp0(tokens[0], "ifilter") == 0) {
            parse_insn_match(tokens[1]);
        } else if (g_strcmp0(tokens[0], "afilter") == 0) {
            parse_vaddr_match(tokens[1]);
        } else {
            fprintf(stderr, "option parsing failed: %s\n", opt);
            return -1;
        }
    }

    /* Register translation block and exit callbacks */
    qemu_plugin_register_vcpu_tb_trans_cb(id, vcpu_tb_trans);
    qemu_plugin_register_atexit_cb(id, plugin_exit, NULL);

    return 0;
}
