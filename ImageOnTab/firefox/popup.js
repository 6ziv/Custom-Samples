$(document).ready(function(){
	var file=$("<input type='file' />");
	file.change(
		function(){
			var reader = new FileReader();
			reader.addEventListener("load",function(){
				chrome.storage.local.set({"image":reader.result});
				browser.tabs.query({},function(tabs){
					for(var tab in tabs){
						browser.tabs.sendMessage(tabs[tab].id, {action:"refresh"});
						if(browser.runtime.lastError){}
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
		browser.tabs.query({active: true, currentWindow: true}, function(tabs) {
			browser.tabs.sendMessage(tabs[0].id, {action:"query"},function(response){
				if(response==undefined){
					if(browser.runtime.lastError){}
					return;
				}
				if(response.show==true)$("#hide").show();
				if(response.show==false)$("#show").show();
			});
			if(browser.runtime.lastError){}
		});
	};
	$("#hide").click(
		function(){
			browser.tabs.query({active: true, currentWindow: true}, function(tabs) {
				browser.tabs.sendMessage(tabs[0].id, {show:false});
				if(browser.runtime.lastError){}
			});
			refresh();
		}
	);
	$("#show").click(
		function(){
			browser.tabs.query({active: true, currentWindow: true}, function(tabs) {
				browser.tabs.sendMessage(tabs[0].id, {show:true});
				if(browser.runtime.lastError){}
			});
			refresh();
		}
	);
	$("#file").click(
		function(){
			alert("Operation not supported in Firefox! Open in tab to select image.");
			file.click();
		}
	);
	$("#opacity").change(
		function(){
			$("#num").val($("#opacity").val());
			chrome.storage.local.set({"opacity":$("#opacity").val()});
			browser.tabs.query({},function(tabs){
				for(var tab in tabs){
					browser.tabs.sendMessage(tabs[tab].id, {action:"refresh"});
					if(browser.runtime.lastError){}
				}
			});
			console.log(1);
		}
	);
	$("#OpenInTab").click(
		function(){
			browser.runtime.openOptionsPage();
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