#!/bin/bash
ps aux | grep "start_service.sh" | grep -v "grep" | awk {'print $2'} | xargs kill $2
ps aux | grep "imageService" | grep -v "grep" | awk {'print $2'} | xargs kill $2
