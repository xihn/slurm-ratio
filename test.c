job_submit_test.c
/*
 * Does some shit
 */

#include <limits.h>
#include <regex.h>
#include <slurm/slurm_errno.h>
#include "src/slurmctld/slurmctld.h"









extern int job_submit(struct job_descriptor *job_desc, uint32_t submit_uid,
        char **err_msg) {
    return _check_ratio(job_desc->partition,
                        job_desc->tres_per_node,
                        job_desc->min_cpus);
}

extern int job_modify(struct job_descriptor *job_desc,
        struct job_record *job_ptr, uint32_t submit_uid) {
    return _check_ratio(
        job_desc->partition == NULL ? job_ptr->partition : job_desc->partition,
        job_desc->tres_per_node == NULL ? job_ptr->tres_per_node : job_desc->tres_per_node,
        job_desc->min_cpus == (uint32_t) -2 ? job_ptr->total_cpus :
             job_desc->min_cpus);
}