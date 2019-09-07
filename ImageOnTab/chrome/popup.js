$(document).ready(function(){
	var file=$("<input type='file' />");
	file.change(
		function(){
			var reader = new FileReader();
			reader.addEventListener("load",function(){
				chrome.storage.local.set({"image":reader.result});
				chrome.tabs.query({},function(tabs){
					for(var tab in tabs){
						chrome.tabs.sendMessage(tabs[tab].id, {action:"refresh"});
					}
				});
				delete reader;
			}, false);
			reader.readAsDataURL(file[0].files[0]);
		}
	);
	var refresh=function(){
		$("#hide").hide();
		$("#show").hide();
		chrome.tabs.query({active: true, currentWindow: true}, function(tabs) {
			chrome.tabs.sendMessage(tabs[0].id, {action:"query"},function(response){
				if(response==undefined){
					if(chrome.runtime.lastError){}
					return;
				}
				if(response.show==true)$("#hide").show();
				if(response.show==false)$("#show").show();
			});
		});
	};
	$("#hide").click(
		function(){
			chrome.tabs.query({active: true, currentWindow: true}, function(tabs) {
				chrome.tabs.sendMessage(tabs[0].id, {show:false});
			});
			refresh();
		}
	);
	$("#show").click(
		function(){
			chrome.tabs.query({active: true, currentWindow: true}, function(tabs) {
				chrome.tabs.sendMessage(tabs[0].id, {show:true});
			});
			refresh();
		}
	);
	$("#file").click(
		function(){
			file.click();
		}
	);
	$("#opacity").change(
		function(){
			$("#num").val($("#opacity").val());
			chrome.storage.local.set({"opacity":$("#opacity").val()});
			chrome.tabs.query({},function(tabs){
				for(var tab in tabs){
					chrome.tabs.sendMessage(tabs[tab].id, {action:"refresh"});
				}
			});			
		}
	);
	$("#OpenInTab").click(
		function(){
			chrome.runtime.openOptionsPage();
		}
	);
	chrome.storage.local.get({"image":""},function(result)
	{
		fetch(result.image).then(function(response) {
			return response.blob();
		}).then(function(blob){
			$("#saver").attr("href",URL.createObjectURL(blob));
			$("#saver").attr("download","image");
			console.log($("#saver").name);
			console.log(blob);
		});
	});

	chrome.storage.local.get({"opacity":10},function(result)
		{
			$("#opacity").val(result.opacity);
			$("#num").val($("#opacity").val());
		}
	);
	refresh();
	
	
});