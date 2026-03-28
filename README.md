# Rail Maintenance - Train Geeks

This project is an implementation of the Rail Maintenance game for the Train Geeks.

## Description

The game simulates a railway system where tracks can become damaged over time. The player controls a maintenance vehicle (drezina) and must repair the tracks to keep trains moving.

The goal is to prevent trains from stopping for too long and keep the railway functional as long as possible.

## Features

- Train system that moves automatically on predefined rails
- Multiple rail segments and stations
- Rail damage system (visual + functional changes)
- Player-controlled maintenance vehicle
- Repair mechanic using interaction key
- Basic collision system

## Gameplay

- Rail segments can become damaged randomly
- Trains stop when they reach a damaged segment
- The player must reach the damaged segment and repair it
- Repairs restore the rail to its original state
- The game ends if:
  - a train waits too long
  - too many segments are damaged at the same time

## Controls

- Movement: WASD
- Repair: F

## Technologies

- C++
- OpenGL

## How to run

Open the project in Visual Studio, build it, and run.

## Notes

This project implements the Rail Maintenance version of the assignment.
