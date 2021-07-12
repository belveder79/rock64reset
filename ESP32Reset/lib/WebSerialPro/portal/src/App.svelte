<script>
	import {
		onMount
	} from 'svelte';

	let host = "ws://"+document.location.host+"/webserialws";
	// let host = "ws://192.168.1.x/webserialws"; // For Local Testing via npm run serve
	const socket = new WebSocket(host);

	let data = {
		connected: false,
		input: '',
		locked: false,
		identity: '',
		monitor: '',
		monitorElement: null
	}

	$: inputIsEmpty = data.input.trim() === '';
	$: monitorIsEmpty = data.monitor === '';

	socket.onopen = function(){
		data.connected = true;
	};

	socket.onclose = function(){
		data.connected = false;
		setTimeout(() => location.reload(), 5000);
	};

	socket.onmessage = function(msg){
		data.monitor += msg.data;
		if(!data.locked){
			data.monitorElement.scrollTop = data.monitorElement.scrollHeight;
		}
	};

	socket.onerror = function(){
		data.connected = false;
		setTimeout(() => location.reload(), 5000);
	};

	async function updateIdentity() {
		const res = await fetch(`/webserial-id`);
		if (res.status === 200) {
			let responseData = await res.json();
			data.identity = responseData.identity;
		}
		return res;
	}

	async function sendMessage() {
		if(data.input.trim() !== ''){
			socket.send(data.input.trim());
			data.input = '';
		}
	}

	function lockMonitor(){
		data.locked = !data.locked;
	}

	function clearMonitor(){
		data.monitor = '';
	}

	onMount(async () => {
		try {
			await updateIdentity();
		} catch (err) {
			console.log(err);
		}
	});
</script>

<div class="container container-xl">
	<div class="row justify-content-center">
		<div class="column text-center">
			<img width="280px" draggable="false" src="data:image/png;base64,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" alt="WebSerialPro" class="logo">
		</div>
	</div>
	{#if !data.connected}
		<div class="row justify-content-center">
			<div class="column main-column column-50">
				<div class="card text-muted">
					<div class="row">
						<div class="loader">
							<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 24 24" style="fill: currentColor"><path d="M12,22c5.421,0,10-4.579,10-10h-2c0,4.337-3.663,8-8,8s-8-3.663-8-8c0-4.336,3.663-8,8-8V2C6.579,2,2,6.58,2,12 C2,17.421,6.579,22,12,22z"></path></svg>
						</div>
					</div>
					<div class="row loader-text">
						<div class="column text-center">
							Connecting
						</div>
					</div>
				</div>
			</div>
		</div>
	{:else}
		<div class="row justify-content-center">
			<div class="column main-column">
				<div class="card text-muted">
					<div class="row">
						<div class="column">
							<input 
								id="messageInput"
								type="text"
								placeholder="Enter here"
								bind:value={data.input}
								on:keydown={(event) => { if(event.key === 'Enter'){ sendMessage() } }}
								tabindex="1"
							>
						</div>
						<div class="column column-25">
							<button class="button full-width" tabindex="2" disabled={inputIsEmpty} on:click={sendMessage}>Send</button>
						</div>
					</div>
					<div class="row">
						<div class="column monitor-container">
							{#if !monitorIsEmpty}
								<div class="monitor-utilities">
									<button class="button clear-button" on:click={clearMonitor}>Clear</button>
									<button class="button lock-button" on:click={lockMonitor}>
										{#if data.locked}
											<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24"><path d="M20,12c0-1.103-0.897-2-2-2h-1V7c0-2.757-2.243-5-5-5S7,4.243,7,7v3H6c-1.103,0-2,0.897-2,2v8c0,1.103,0.897,2,2,2h12 c1.103,0,2-0.897,2-2V12z M9,7c0-1.654,1.346-3,3-3s3,1.346,3,3v3H9V7z"></path></svg>
										{:else}
											<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24"><path d="M17,8V7c0-2.757-2.243-5-5-5S7,4.243,7,7v3H6c-1.103,0-2,0.897-2,2v8c0,1.103,0.897,2,2,2h12c1.103,0,2-0.897,2-2v-8 c0-1.103-0.897-2-2-2H9V7c0-1.654,1.346-3,3-3s3,1.346,3,3v1H17z M18,12l0.002,8H6v-8H18z"></path></svg>
										{/if}	
									</button>
								</div>
							{/if}
							<textarea disabled="disabled" class="monitor" bind:value={data.monitor} bind:this={data.monitorElement}></textarea>
						</div>
					</div>
				</div>
			</div>
		</div>
		<div class="row pill-row">
			<div class="column text-center">
				<span class="pill">{data.identity}</span>
			</div>
		</div>
	{/if}
</div>

<style type="text/scss" global>
	$color-primary: #28D6B9;
	$color-secondary: #506690;
	$color-muted: #869ab8;
	$color-quinary: #869ab8;

	@import "../node_modules/milligram/src/milligram.sass";

	.text-muted{
		color: $color-muted !important;
	}

	.container-xl{
		margin: 4rem auto;
	}

	.main-column{
		flex: 0 0 85%;
		max-width: 85%;

		@media (min-width: 40rem){
			flex: 0 0 75% !important;
			max-width: 75% !important;
		}

		@media (min-width: 60rem){
			flex: 0 0 50% !important;
			max-width: 50% !important;
		}
	}

	.justify-content-center{
		justify-content: center;
	}

	.text-center{
		text-align: center;
	}

	input{
		transition: border .15s linear;
	}

	input::placeholder{
		color: $color-quinary;
	}

	.monitor-container{
		position: relative;
	}

	.monitor{
		white-space: pre-wrap;
		height: 200px;
		width: 100%;
		margin-bottom: 1rem;
		resize: vertical;
		background-color: $color-tertiary;
		border-width: 0px;
	}

	.monitor-utilities{
		position: absolute;
		top: 20px;
		right: 40px;
		-webkit-animation: fadeInDown .5s linear;
		-moz-animation: fadeInDown .5s linear;
		animation: fadeInDown .5s linear;
		
		@media (max-width: 40rem){
			top: 10px;
			right: 10px;
		}
	}

	.clear-button{
		font-size: 0.8rem;
    height: auto;
		line-height: 2.8rem;
		background-color: #506690;
		border-color: #506690;
		&:not(:first-child){
			margin-left: .55rem;
		}
	}

	.lock-button{
		font-size: 0.8rem;
    height: auto;
		line-height: 2.8rem;
		background-color: #506690;
		border-color: #506690;
		&:not(:first-child){
			margin-left: .55rem;
		}
		svg {
			fill: currentColor;
			vertical-align: sub;
			padding-top: 4px;
		}
	}

	@media (max-width: 40rem){
		.row{
			flex-direction: row !important;
			.column:not(:first-child){
				padding-left: 1.0rem;
			}
		}
		.card{
			padding: 1.55rem 1.55rem !important;
		}
	}

	.pill-row{
		margin: 4rem auto;
	}

	.pill{
		padding: .55rem .75rem;
		font-size: 1.1rem;
		background-color: $color-primary;
		border-radius: 25px;
		min-width: 60px;
		text-align: center;
		color: #fff;
	}

	.card {
		box-shadow: 0 .5rem 1.5rem rgba(22,28,45,.1)!important;
		border: 0 solid #f1f4f8;
		border-radius: .575rem;
		padding: 2rem 2.55rem;
		width: 100%;
	}

	.logo{
		margin: 1rem auto 4rem auto;
		user-select: none;
	}

	.loader {
		margin: 4rem auto 2rem auto;
		svg {
			vertical-align: middle;
			-webkit-animation:spin 1.5s linear infinite;
			-moz-animation:spin 1.5s linear infinite;
			animation:spin 1.5s linear infinite;
		}
	}

	.loader-text{
		margin-bottom: 2rem;
	}

	.button {
		outline: none;
		padding: 0 1rem;
		transition: background-color .15s linear, border .15s linear;

		&.full-width{
			width: 100%
		}

		&.loading svg{
			-webkit-animation:spin 1.5s linear infinite;
			-moz-animation:spin 1.5s linear infinite;
			animation:spin 1.5s linear infinite;
		}
	}

	@-moz-keyframes spin { 100% { -moz-transform: rotate(360deg); } }
	@-webkit-keyframes spin { 100% { -webkit-transform: rotate(360deg); } }
	@keyframes spin { 100% { -webkit-transform: rotate(360deg); transform:rotate(360deg); } }

	@keyframes fadeInDown {
		0% {
				opacity: 0;
				transform: translateY(-5px);
		}
		100% {
				opacity: 1;
				transform: translateY(0);
		}
	} 
</style>
