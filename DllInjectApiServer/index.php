<?php

require("database.php");

$cmd = $_REQUEST["command"];

PaiDatabase::connectDB();

function error($msg)
{
    PaiDatabase::disconnectDB();
    header('Content-Type: application/json');
    echo json_encode(array('status' => "error", 'msg' => $msg));
}

function success($msg)
{
    PaiDatabase::disconnectDB();
    header('Content-Type: application/json');
    echo json_encode(array('status' => "success", 'msg' => $msg));
}

if ($cmd == "save_inject_data") {
    $IPAddress = $_SERVER['REMOTE_ADDR'];
    $HWID = $_REQUEST["HWID"];
    $Pattern = $_REQUEST["Pattern"];

    $res = PaiDatabase::executeSQL("INSERT INTO inject_store(ip_address, hwid, pattern, created_at) VALUES ('$IPAddress', '$HWID', '$Pattern', CURRENT_TIME())");
    if ($res)
        success("Success to insert into database.");
    else
        error("Failed to insert into database.");
}