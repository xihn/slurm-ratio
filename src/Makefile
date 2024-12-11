# Paths
SLURM_INC = /usr/include/slurm
SLURM_SRC = /usr/local/src/slurm-src
SLURM_LIB = /usr/lib64

# Compiler and Flags
CC = gcc
CFLAGS = -D_GNU_SOURCE -fPIC -shared -I$(SLURM_INC) -I$(SLURM_SRC) -Wall
LDFLAGS = -L$(SLURM_LIB) -lslurm

# Target
PLUGIN = job_submit_require_cpu_gpu_ratio.so
SRC = job_submit_require_cpu_gpu_ratio.c

# Build the plugin
all: $(PLUGIN)

$(PLUGIN): $(SRC)
        $(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Clean up generated files
clean:
        rm -f $(PLUGIN)
