<?php

echo "to lock\n";
wcx_lock();
echo "to locked\n";
sleep(30);
echo "to unlock\n";
wcx_unlock();
echo "to unlocked\n";

