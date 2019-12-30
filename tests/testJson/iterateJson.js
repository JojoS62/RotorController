
var Response = {"current_size": 35097, "max_size": 37811, "alloc_cnt": 54, "reserved_size": 82664};
//var jsonResponse = JSON.parse(Response);

for(var k in Response)
{
    console.log(k);
    console.log(Response[k]);
    //console.log(Object.keys(k));
}

console.log(Object.values(Response));