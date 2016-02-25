Pebble.addEventListener('ready', function(e) {
  console.log('JavaScript app ready and running!');
});

Pebble.addEventListener('appmessage',
  function(e) {
    console.log(JSON.stringify(e.payload));
		
		var xhttp = new XMLHttpRequest();
		xhttp.open("GET","http://sleeptrack.developer-zone.ch/log.php?id=" + JSON.stringify(e.payload) ,true);
		xhttp.send();
  }
);