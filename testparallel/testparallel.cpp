#include "apphost.h"
#include "event.h"

#include "enkiTS/TaskScheduler.h"

using namespace enki;

TaskScheduler g_TS;
uint32_t g_Iteration;

std::atomic<int32_t> g_WaitForTaskCompletion(0);
std::atomic<int32_t> g_WaitCount(0);

struct SlowTask : enki::ITaskSet
{
    void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override
    {
        // fake slow task with timer
        Event e;
        e.wait(waitTime);
    }

    double waitTime;
};

struct SlowPinnedTask : enki::IPinnedTask
{
    void Execute() override
    {
        // fake slow task with timer
        Event e;
        e.wait(waitTime);
    }

    double waitTime;
};

struct WaitingTask : enki::ITaskSet
{
    void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override
    {
        int numWaitTasks = maxWaitasks - depth;
        for (int t = 0; t < numWaitTasks; ++t)
        {
            pWaitingTasks[t] = new WaitingTask;
            pWaitingTasks[t]->depth = depth + 1;
            g_TS.AddTaskSetToPipe(pWaitingTasks[t]);
        }
        for (SlowTask& task : tasks)
        {
            task.m_SetSize = 1000;
            task.waitTime = 0.00001 * double(rand() % 100);
            g_TS.AddTaskSetToPipe(&task);
        }

        for (SlowTask& task : tasks)
        {
            ++g_WaitCount;
            g_TS.WaitforTask(&task);

            // we add a random wait for pinned task here.
            uint32_t randThread = (uint32_t)rand() % g_TS.GetNumTaskThreads();
            pinnedTask.threadNum = randThread;
            pinnedTask.waitTime = 0.00001 * double(rand() % 10);
            g_TS.AddPinnedTask(&pinnedTask);
            g_TS.WaitforTask(&pinnedTask);
        }

        for (int t = 0; t < numWaitTasks; ++t)
        {
            ++g_WaitCount;
            g_TS.WaitforTask(pWaitingTasks[t]);
        }
    }

    virtual ~WaitingTask()
    {
        for (WaitingTask* pWaitingTask : pWaitingTasks)
        {
            delete pWaitingTask;
        }
    }
    SlowTask    tasks[4];
    SlowPinnedTask pinnedTask;
    int32_t     depth = 0;
    static constexpr int maxWaitasks = 4;
    WaitingTask* pWaitingTasks[maxWaitasks] = {};
};

void onLoad()
{
    enki::TaskSchedulerConfig config;
    config.profilerCallbacks.waitForTaskCompleteStart = [](uint32_t threadnum_) { ++g_WaitCount; };
    config.profilerCallbacks.waitForTaskCompleteSuspendStart = [](uint32_t threadnum_) { ++g_WaitForTaskCompletion; };
    g_TS.Initialize(config);
    for (g_Iteration = 0; g_Iteration < 1000; ++g_Iteration)
    {
        printf("\tIteration %d: Waits: %d blocking waits: %d\n",
            g_Iteration, g_WaitCount.load(), g_WaitForTaskCompletion.load());
        WaitingTask taskRoot;
        g_TS.AddTaskSetToPipe(&taskRoot);
        g_TS.WaitforAll();
    }
}