-- Mock slurm environment for testing
slurm = {}
slurm.SUCCESS = 0
slurm.ERROR = 1
slurm.ESLURM_INVALID_GRES = 2

function slurm.log_info(msg)
    print("[INFO] " .. msg)
end

function slurm.log_user(msg)
    print("[USER] " .. msg)
end

-- Begin the plugin code
myname = "job_submit_require_cpu_gpu_ratio"
enabled = true
partition = "es1"
default_ratio = 2.0
epsilon = 1e-9

local card_ratios = {
    gtrx2080ti = 2.0,
    v100       = 2.0,
    a100       = 4.0,
    h100       = 6.0
}

local function are_equal(a, b)
    return math.abs(a - b) < epsilon
end

local function check_single_gpu_request(ncpu, gpu_name, gpu_count)
    local required_ratio = default_ratio
    if gpu_name and card_ratios[gpu_name] then
        required_ratio = card_ratios[gpu_name]
    elseif gpu_name then
        slurm.log_info(myname .. ": Missing ratio info for GPU '" .. gpu_name ..
                       "', using default ratio " .. default_ratio)
    else
        slurm.log_info(myname .. ": No GPU name specified, using default ratio " .. default_ratio)
    end

    local ratio = ncpu / gpu_count
    if not are_equal(ratio, required_ratio) then
        slurm.log_user("Error: Requested CPU/GPU ratio " .. ratio ..
                       " does not match required ratio " .. required_ratio)
        return slurm.ESLURM_INVALID_GRES
    end

    return slurm.SUCCESS
end

local function parse_and_check_gpu_requests(part, tres, ncpu)
    if part ~= partition then
        return slurm.SUCCESS
    end

    if not tres or tres == "" then
        slurm.log_info(myname .. ": No GRES specified on partition " .. part .. ", skipping ratio checks.")
        return slurm.SUCCESS
    end

    for entry in string.gmatch(tres, "([^+]+)") do
        local lower_entry = string.lower(entry)
        local gpu_name, gpu_count_str = string.match(lower_entry, "^gpu:([a-z0-9]+):(%d+)$")
        local gpu_count

        if gpu_name and gpu_count_str then
            gpu_count = tonumber(gpu_count_str)
            if not gpu_count or gpu_count <= 0 then
                slurm.log_user(myname .. ": Invalid GPU count '" .. tostring(gpu_count_str) ..
                               "' in request '" .. entry .. "'")
                return slurm.ESLURM_INVALID_GRES
            end
        else
            local gpu_count_str_only = string.match(lower_entry, "^gpu:(%d+)$")
            if gpu_count_str_only then
                gpu_count = tonumber(gpu_count_str_only)
                gpu_name = nil
                if not gpu_count or gpu_count <= 0 then
                    slurm.log_user(myname .. ": Invalid GPU count '" .. tostring(gpu_count_str_only) ..
                                   "' in request '" .. entry .. "'")
                    return slurm.ESLURM_INVALID_GRES
                end
            else
                slurm.log_user(myname .. ": Invalid GPU format in TRES '" .. entry .. "'")
                return slurm.ESLURM_INVALID_GRES
            end
        end

        local rc = check_single_gpu_request(ncpu, gpu_name, gpu_count)
        if rc ~= slurm.SUCCESS then
            return rc
        end
    end

    return slurm.SUCCESS
end

function slurm_job_submit(job_desc, part_list, submit_uid)
    if not enabled then
        return slurm.SUCCESS
    end

    local part = job_desc.partition or ""
    local tres = job_desc.tres_per_node
    local ncpu = job_desc.min_cpu or 1

    local result = parse_and_check_gpu_requests(part, tres, ncpu)
    -- Print the result for demonstration
    if result == slurm.SUCCESS then
        print("Result: slurm.SUCCESS")
    elseif result == slurm.ESLURM_INVALID_GRES then
        print("Result: slurm.ESLURM_INVALID_GRES")
    else
        print("Result: slurm.ERROR")
    end

    return result
end

-- Test runner

local function test(description, job_desc)
    print("\n--- " .. description .. " ---")
    local result = slurm_job_submit(job_desc, {}, 1000)
    print("Test completed with result code: " .. result)
end

-- 1. No GPUs requested, in target partition
test("No GPUs, target partition", {
    partition = "es1",
    min_cpu = 4,
    tres_per_node = nil
})
-- Expect: SUCCESS (no GPUs requested, ratio not enforced)

-- 2. No GPUs requested, different partition
test("No GPUs, different partition", {
    partition = "other",
    min_cpu = 4,
    tres_per_node = nil
})
-- Expect: SUCCESS (not enforcing on this partition)

-- 3. GPU with name that is known and exact ratio
-- Known ratio for v100 is 2.0, request: min_cpu=4, gpu:v100:2 means ratio=4/2=2.0 matches
test("Known GPU name, exact ratio", {
    partition = "es1",
    min_cpu = 4,
    tres_per_node = "gpu:v100:2"
})
-- Expect: SUCCESS

-- 4. GPU with name that is known but wrong ratio
-- Known ratio for a100 is 4.0, request: min_cpu=8, gpu:a100:1 ratio=8/1=8 != 4
test("Known GPU name, incorrect ratio", {
    partition = "es1",
    min_cpu = 8,
    tres_per_node = "gpu:a100:1"
})
-- Expect: ESLURM_INVALID_GRES

-- 5. GPU without a name, use default ratio
-- default_ratio=2.0, request: min_cpu=4, gpu:2 ratio=4/2=2.0 matches default
test("No GPU name, default ratio matches", {
    partition = "es1",
    min_cpu = 4,
    tres_per_node = "gpu:2"
})
-- Expect: SUCCESS

-- 6. GPU without a name, ratio not matching
-- default_ratio=2.0, request: min_cpu=6, gpu:2 ratio=6/2=3.0 != 2.0
test("No GPU name, default ratio does not match", {
    partition = "es1",
    min_cpu = 6,
    tres_per_node = "gpu:2"
})
-- Expect: ESLURM_INVALID_GRES

-- 7. Unknown GPU name, fallback to default
-- unknown GPU "abc": default_ratio=2.0
-- request: min_cpu=6, gpu:abc:3 ratio=6/3=2.0 matches default
test("Unknown GPU name, fallback to default ratio matches", {
    partition = "es1",
    min_cpu = 6,
    tres_per_node = "gpu:abc:3"
})
-- Expect: SUCCESS (info logged about using default ratio)

-- 8. Unknown GPU name, fallback to default ratio not matching
-- unknown GPU "xyz": default_ratio=2.0
-- request: min_cpu=10, gpu:xyz:4 ratio=10/4=2.5 != 2.0
test("Unknown GPU name, fallback to default ratio does not match", {
    partition = "es1",
    min_cpu = 10,
    tres_per_node = "gpu:xyz:4"
})
-- Expect: ESLURM_INVALID_GRES

-- 9. Multiple GPU specs, all correct
-- partition=es1, min_cpu=10
-- gpu:v100:2 (ratio=10/2=5, but v100 ratio=2.0 - this is a fail)
-- Let's correct this example to be consistent:
-- Let's pick min_cpu=8
-- gpu:v100:4 ratio=8/4=2.0 matches v100
-- gpu:2 ratio=8/2=4.0 with no name (default=2.0) - this will fail actually
-- We need both to pass:
-- Let's try min_cpu=8
-- gpu:v100:4 ratio=8/4=2.0 matches v100's 2.0
-- gpu:a100:2 ratio=8/2=4.0 matches a100's 4.0
test("Multiple GPUs all correct ratios", {
    partition = "es1",
    min_cpu = 8,
    tres_per_node = "gpu:v100:4+gpu:a100:2"
})
-- Expect: SUCCESS

-- 10. Multiple GPU specs, one fails
-- min_cpu=8
-- gpu:v100:4 ratio=2.0 matches v100
-- gpu:a100:1 ratio=8.0 does not match a100's 4.0
test("Multiple GPUs one fails", {
    partition = "es1",
    min_cpu = 8,
    tres_per_node = "gpu:v100:4+gpu:a100:1"
})
-- Expect: ESLURM_INVALID_GRES

-- 11. Invalid format
test("Invalid GPU format", {
    partition = "es1",
    min_cpu = 4,
    tres_per_node = "gpu:::2"
})
-- Expect: ESLURM_INVALID_GRES

-- 12. Invalid count
test("Invalid count", {
    partition = "es1",
    min_cpu = 4,
    tres_per_node = "gpu:-1"
})
-- Expect: ESLURM_INVALID_GRES
