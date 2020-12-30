/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Wqueue.hpp
 * Author: mahdi
 *
 * Created on January 9, 2018, 11:42 AM
 */

#ifndef WQUEUE_HPP
#define WQUEUE_HPP

#include <pthread.h>
#include <list>
#include <stdio.h>
#include <stdlib.h>

template <typename T> class Wqueue
{ 
    std::list<T>   m_queue;
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_condv;

public:
    Wqueue() {
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_condv, NULL);
    }
    ~Wqueue() {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_condv);
    }
    void enqueue(T item) {
        pthread_mutex_lock(&m_mutex);
        m_queue.push_back(item);
        pthread_cond_signal(&m_condv);
        pthread_mutex_unlock(&m_mutex);
    }
    T dequeue() {
        pthread_mutex_lock(&m_mutex);
        while (m_queue.size() == 0) {
            pthread_cond_wait(&m_condv, &m_mutex);
        }
        T item = m_queue.front();
        m_queue.pop_front();
        pthread_mutex_unlock(&m_mutex);
        return item;
    }
    int size() {
        pthread_mutex_lock(&m_mutex);
        int size = m_queue.size();
        pthread_mutex_unlock(&m_mutex);
        return size;
    }
};

#endif /* WQUEUE_HPP */

