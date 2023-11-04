#include "TaskQueue.h"

TaskQueue::TaskQueue(){
    pthread_mutex_init(&m_mutex, nullptr);
}

TaskQueue::~TaskQueue(){
    pthread_mutex_destroy(&m_mutex);
}

void TaskQueue::addTask(Task task){
    pthread_mutex_lock(&m_mutex);
    m_taskQ.push(task);
    pthread_mutex_unlock(&m_mutex);
}

void TaskQueue::addTask(callback f, void* arg){
    pthread_mutex_lock(&m_mutex);
    m_taskQ.push(Task(f, arg));
    pthread_mutex_unlock(&m_mutex);
}

Task TaskQueue::takeTask(){
    Task t;
    pthread_mutex_lock(&m_mutex);
    if (!m_taskQ.empty()) {
        t = m_taskQ.front();
        m_taskQ.pop();
    }
    pthread_mutex_unlock(&m_mutex);
    return t;
}