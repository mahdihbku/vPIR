/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Wqueue.cpp
 * Author: mahdi
 * 
 * Created on January 9, 2018, 11:42 AM
 */

#include <pthread.h>
#include <list>
#include <stdio.h>
#include <stdlib.h>

#include "Wqueue.hpp"

using namespace std;

template <typename T> Wqueue<T>::Wqueue() {
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_condv, NULL);
}

template <typename T> Wqueue<T>::Wqueue(const Wqueue& orig) {
}

template <typename T> Wqueue<T>::~Wqueue() {
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_condv);
}

template <typename T> int Wqueue<T>::enqueue(T item) {
    pthread_mutex_lock(&m_mutex);
    m_queue.push_back(item);
    pthread_cond_signal(&m_condv);
    pthread_mutex_unlock(&m_mutex);
}

template <typename T> T Wqueue<T>::dequeue() {
    pthread_mutex_lock(&m_mutex);
    while (m_queue.size() == 0) {
        pthread_cond_wait(&m_condv, &m_mutex);
    }
    T item = m_queue.front();
    m_queue.pop_front();
    pthread_mutex_unlock(&m_mutex);
    return item;
}

template <typename T> int Wqueue<T>::size() {
    pthread_mutex_lock(&m_mutex);
    int size = m_queue.size();
    pthread_mutex_unlock(&m_mutex);
    return size;
}

