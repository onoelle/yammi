#!/bin/bash
# test starting yammi without any database info

echo "moving database"

echo "starting yammi without existing database..."

mv ~/.kde/share/apps/yammi ~/.kde/share/apps/yammi_backup_smoketest

src/yammi

echo "yammi returned control, press return when you want to start yammi again"
echo "[PRESS RETURN]"
read answer   # read from keyboard

echo "starting yammi another time, with the newly created database existing"

src/yammi
echo "yammi returned control, press return when you want to finish this test"
echo "[PRESS RETURN]"
read answer   # read from keyboard


echo "deleting newly created database..."
rm -r ~/.kde/share/apps/yammi
echo "restoring backup of data..."
mv ~/.kde/share/apps/yammi_backup_smoketest ~/.kde/share/apps/yammi
echo "test finished!"
