#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

struct Process {
    int id;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;
    int original_priority;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    bool started;
};

void resetProcesses(struct Process p[], struct Process original[], int n) {
    for (int i = 0; i < n; i++) {
        p[i] = original[i];
        p[i].remaining_time = p[i].burst_time;
        p[i].started = false;
        p[i].response_time = -1;
    }
}

// 1. FCFS (First-Come, First-Served)
void runFCFS(struct Process p[], int n) {
    // Sort by arrival time
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (p[j].arrival_time > p[j + 1].arrival_time) {
                struct Process temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;
            }
        }
    }

    int current_time = 0;
    printf("\n--- FCFS Gantt Chart ---\n| ");
    for (int i = 0; i < n; i++) {
        if (current_time < p[i].arrival_time) {
            printf("IDLE (%d-%d) | ", current_time, p[i].arrival_time);
            current_time = p[i].arrival_time;
        }
        p[i].response_time = current_time - p[i].arrival_time;
        current_time += p[i].burst_time;
        p[i].completion_time = current_time;
        p[i].turnaround_time = p[i].completion_time - p[i].arrival_time;
        p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;

        printf("P%d (%d) | ", p[i].id, p[i].completion_time);
    }
    printf("\n");
}

// 2. SJF (Non-Preemptive)
void runSJF(struct Process p[], int n) {
    int completed = 0, current_time = 0;
    bool is_completed[10] = {false};

    printf("\n--- SJF (Non-Preemptive) Gantt Chart ---\n| ");
    while (completed < n) {
        int idx = -1;
        int min_burst = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && !is_completed[i]) {
                if (p[i].burst_time < min_burst) {
                    min_burst = p[i].burst_time;
                    idx = i;
                }
                if (p[i].burst_time == min_burst) {
                    if (p[i].arrival_time < p[idx].arrival_time) {
                        idx = i;
                    }
                }
            }
        }

        if (idx == -1) {
            current_time++;
            continue;
        }

        p[idx].response_time = current_time - p[idx].arrival_time;
        current_time += p[idx].burst_time;
        p[idx].completion_time = current_time;
        p[idx].turnaround_time = p[idx].completion_time - p[idx].arrival_time;
        p[idx].waiting_time = p[idx].turnaround_time - p[idx].burst_time;
        is_completed[idx] = true;
        completed++;

        printf("P%d (%d) | ", p[idx].id, p[idx].completion_time);
    }
    printf("\n");
}

// 3. SRTF (Shortest Remaining Time First - Preemptive)
void runSRTF(struct Process p[], int n) {
    int completed = 0, current_time = 0;
    int prev = -1;

    printf("\n--- SRTF Gantt Chart ---\n| ");
    while (completed < n) {
        int idx = -1;
        int min_rem = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && p[i].remaining_time > 0) {
                if (p[i].remaining_time < min_rem) {
                    min_rem = p[i].remaining_time;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            current_time++;
            continue;
        }

        if (!p[idx].started) {
            p[idx].response_time = current_time - p[idx].arrival_time;
            p[idx].started = true;
        }

        p[idx].remaining_time--;
        current_time++;

        if (prev != idx) {
            printf(" P%d ", p[idx].id);
            prev = idx;
        }

        if (p[idx].remaining_time == 0) {
            p[idx].completion_time = current_time;
            p[idx].turnaround_time = p[idx].completion_time - p[idx].arrival_time;
            p[idx].waiting_time = p[idx].turnaround_time - p[idx].burst_time;
            completed++;
        }
    }
    printf(" | \n");
}

// 4. Round Robin
void runRoundRobin(struct Process p[], int n, int quantum) {
    int current_time = 0, completed = 0;
    int queue[100], front = 0, rear = 0;
    bool in_queue[10] = {false};

    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].burst_time;
    }

    // Sort by arrival time initially
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (p[j].arrival_time > p[j + 1].arrival_time) {
                struct Process temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;
            }
        }
    }

    queue[rear++] = 0;
    in_queue[0] = true;

    printf("\n--- Round Robin (Q=%d) Gantt Chart ---\n| ", quantum);
    while (front < rear) {
        int idx = queue[front++];
        
        if (!p[idx].started) {
            p[idx].response_time = current_time - p[idx].arrival_time;
            p[idx].started = true;
        }

        int exec_time = (p[idx].remaining_time > quantum) ? quantum : p[idx].remaining_time;
        p[idx].remaining_time -= exec_time;
        current_time += exec_time;

        // Push newly arrived processes
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && p[i].remaining_time > 0 && !in_queue[i]) {
                queue[rear++] = i;
                in_queue[i] = true;
            }
        }

        if (p[idx].remaining_time > 0) {
            queue[rear++] = idx;
        } else {
            p[idx].completion_time = current_time;
            p[idx].turnaround_time = p[idx].completion_time - p[idx].arrival_time;
            p[idx].waiting_time = p[idx].turnaround_time - p[idx].burst_time;
            completed++;
        }

        printf("P%d (%d) | ", p[idx].id, current_time);
    }
    printf("\n");
}

// 5. Priority Scheduling with Aging (Prevent Starvation)
void runPriorityWithAging(struct Process p[], int n) {
    int completed = 0, current_time = 0;
    bool is_completed[10] = {false};

    for (int i = 0; i < n; i++) {
        p[i].priority = p[i].original_priority;
    }

    printf("\n--- Priority Scheduling (with Aging) Gantt Chart ---\n| ");
    while (completed < n) {
        // Aging mechanism: Increase priority of waiting processes
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && !is_completed[i]) {
                p[i].priority--; // Higher priority value means lower priority, reducing it increases priority
            }
        }

        int idx = -1;
        int highest_priority = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && !is_completed[i]) {
                if (p[i].priority < highest_priority) {
                    highest_priority = p[i].priority;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            current_time++;
            continue;
        }

        if (!p[idx].started) {
            p[idx].response_time = current_time - p[idx].arrival_time;
            p[idx].started = true;
        }

        current_time++;
        p[idx].remaining_time--;

        if (p[idx].remaining_time == 0) {
            p[idx].completion_time = current_time;
            p[idx].turnaround_time = p[idx].completion_time - p[idx].arrival_time;
            p[idx].waiting_time = p[idx].turnaround_time - p[idx].burst_time;
            is_completed[idx] = true;
            completed++;
        }

        printf(" P%d ", p[idx].id);
    }
    printf(" | \n");
}

void printTable(struct Process p[], int n) {
    printf("\nID\tArrival\tBurst\tPriority\tCompletion\tTurnaround\tWaiting\tResponse\n");
    float total_wt = 0, total_tat = 0, total_rt = 0;
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t\t%d\t\t%d\t\t%d\t%d\n", 
            p[i].id, p[i].arrival_time, p[i].burst_time, p[i].original_priority,
            p[i].completion_time, p[i].turnaround_time, 
            p[i].waiting_time, p[i].response_time);
        total_wt += p[i].waiting_time;
        total_tat += p[i].turnaround_time;
        total_rt += p[i].response_time;
    }
    printf("\nAverage Waiting Time: %.2f", total_wt / n);
    printf("\nAverage Turnaround Time: %.2f", total_tat / n);
    printf("\nAverage Response Time: %.2f\n", total_rt / n);
}

int main() {
    int n, quantum;
    printf("Enter number of processes: ");
    scanf("%d", &n);

    struct Process p[10], original[10];

    for (int i = 0; i < n; i++) {
        original[i].id = i + 1;
        printf("\nProcess P%d:\n", original[i].id);
        printf("Arrival Time: ");
        scanf("%d", &original[i].arrival_time);
        printf("Burst Time: ");
        scanf("%d", &original[i].burst_time);
        printf("Priority (lower number = higher priority): ");
        scanf("%d", &original[i].original_priority);
        original[i].remaining_time = original[i].burst_time;
        original[i].started = false;
        original[i].response_time = -1;
    }

    printf("Enter Time Quantum for Round Robin: ");
    scanf("%d", &quantum);

    // 1. FCFS
    resetProcesses(p, original, n);
    runFCFS(p, n);
    printTable(p, n);

    // 2. SJF
    resetProcesses(p, original, n);
    runSJF(p, n);
    printTable(p, n);

    // 3. SRTF
    resetProcesses(p, original, n);
    runSRTF(p, n);
    printTable(p, n);

    // 4. Round Robin
    resetProcesses(p, original, n);
    runRoundRobin(p, n, quantum);
    printTable(p, n);

    // 5. Priority with Aging
    resetProcesses(p, original, n);
    runPriorityWithAging(p, n);
    printTable(p, n);

    return 0;
}