#!/bin/bash

APP="./de_mucholoko"  
ARG1="check"
ARG2=$(python3 keygen.py)

"$APP" "$ARG1" "$ARG2"
