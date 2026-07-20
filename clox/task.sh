#!/bin/bash

# Quick tasks script for clox
# Usage: ./task.sh {a|b|c|d}

case "$1" in
    "a")
        echo "Executing task A..."
        # Add your task here
        ;;
    "b")
        echo "Executing task B..."
        # Add your task here
        ;;
    "c")
        echo "Executing task C..."
        # Add your task here
        ;;
    "d")
        echo "Executing task D..."
        # Add your task here
        ;;
    *)
        echo "Usage: $0 {a|b|c|d}"
        echo "  a - Task A"
        echo "  b - Task B"
        echo "  c - Task C"
        echo "  d - Task D"
        exit 1
        ;;
esac
