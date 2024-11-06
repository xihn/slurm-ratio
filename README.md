# slurm-ratio

Job Submit plugin to enfornce the CPU/GPU ratio to facilitate accounting.

### Setup
1. Clone the repo
   ```sh
   git clone https://github.com/xihn/slurm-ratio.git
   ```
3. Configure `job_submit_ratio_config.yml` to your own needs. See the [configuration instructions](#Configuration)
4. Make sure the name of your configuration file matches `configFile` in line 51
5. Compile 
   ```sh
   gcc -shared -fPIC -pthread -I${SLURM_SRC_DIR} job_submit_require_cpu_gpu_ratio.c -o job_submit_require_cpu_gpu_ratio.so
   ```

### Testing

For testing without compiling slurm. `cd src/print` and run `gcc ratio_print.c` (or compile using any c11 compiler). Then run `./a.out "partition" "gpu:type:count" "cpu count"` with quotations included. 


`scontrol show job ID`

`scontrol show nodes | grep -E 'NodeName=|Gres=' | grep -B 1 'Gres=[gpu:]'`

`scontrol show node n0001.es1`
