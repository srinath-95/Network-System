HTTP/1.1 200 OK
Date: Wed, 05 Dec 2018 02:03:11 GMT
Server: Apache/2.4.6 (CentOS) OpenSSL/1.0.1e-fips PHP/5.6.31
Last-Modified: Wed, 09 Jan 2002 09:40:05 GMT
ETag: "3bf-3971b7a7c2740"
Accept-Ranges: bytes
Content-Length: 959
Content-Type: application/javascript

var ourInterval;
var origColor = "#191919";
var overColor = "#191919";
var scrollSpeed = 50;
var scrollHeight = 5;

function scrollStart(direction, divID, elementID){
//CHANGE THE BACKGROUND COLOR OF THE TD THE MOUSE IS OVER
document.getElementById(elementID).style.backgroundColor = overColor;
// REPEATED CALL EITHER scrollUp OR scrollDown
ourInterval = setInterval("scroll"+direction+"('"+divID+"')", scrollSpeed);
}
function scrollEnd(which){
// OUR MOUSE IS OUT, SO RETURN TD TO ORIGINAL COLOR
document.getElementById(which).style.backgroundColor = origColor;
// STOP CALLING THE SCROLL FUNCTION
clearInterval(ourInterval);
}

function scrollUp(which){
// SET THE SCROLL TOP
document.getElementById(which).scrollTop = document.getElementById(which).scrollTop - scrollHeight;
}
function scrollDown(which){
// SET THE SCROLL TOP
document.getElementById(which).scrollTop = document.getElementById(which).scrollTop + scrollHeight;
}