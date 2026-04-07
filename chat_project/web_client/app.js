const state = {
  socket: null,
  currentUser: "",
  currentPeer: "",
  conversations: {},
  historyLoadedPeers: {},
  onlineUsers: [],
};

const els = {
  authShell: document.getElementById("authShell"),
  mainShell: document.getElementById("mainShell"),
  wsUrl: document.getElementById("wsUrl"),
  toggleSettingsBtn: document.getElementById("toggleSettingsBtn"),
  settingsPanel: document.getElementById("settingsPanel"),
  loginUsername: document.getElementById("loginUsername"),
  loginPassword: document.getElementById("loginPassword"),
  registerUsername: document.getElementById("registerUsername"),
  registerPassword: document.getElementById("registerPassword"),
  connectBtn: document.getElementById("connectBtn"),
  registerBtn: document.getElementById("registerBtn"),
  loginBtn: document.getElementById("loginBtn"),
  showLoginBtn: document.getElementById("showLoginBtn"),
  showRegisterBtn: document.getElementById("showRegisterBtn"),
  loginCard: document.getElementById("loginCard"),
  registerCard: document.getElementById("registerCard"),
  refreshBtn: document.getElementById("refreshBtn"),
  exportBtn: document.getElementById("exportBtn"),
  sendBtn: document.getElementById("sendBtn"),
  messageInput: document.getElementById("messageInput"),
  statusText: document.getElementById("statusText"),
  onlineUsers: document.getElementById("onlineUsers"),
  conversationList: document.getElementById("conversationList"),
  messages: document.getElementById("messages"),
  chatTitle: document.getElementById("chatTitle"),
  chatSub: document.getElementById("chatSub"),
  currentUserBadge: document.getElementById("currentUserBadge"),
};

function setStatus(text) {
  els.statusText.textContent = text;
}

function connectGateway() {
  const url = els.wsUrl.value.trim();
  if (!url) {
    setStatus("请输入服务器地址。");
    return;
  }

  if (state.socket && state.socket.readyState === WebSocket.OPEN) {
    setStatus("已连接。");
    return;
  }

  state.socket = new WebSocket(url);
  setStatus("正在连接...");

  state.socket.onopen = () => {
    setStatus("连接成功。");
  };

  state.socket.onclose = () => {
    setStatus("连接已断开。");
  };

  state.socket.onerror = () => {
    setStatus("连接失败，请检查服务是否已启动。");
  };

  state.socket.onmessage = (event) => {
    try {
      const data = JSON.parse(event.data);
      handleServerMessage(data);
    } catch (error) {
      setStatus("收到无法解析的消息。");
    }
  };
}

function sendJson(obj) {
  if (!state.socket || state.socket.readyState !== WebSocket.OPEN) {
    setStatus("尚未连接到网关。");
    return false;
  }
  state.socket.send(JSON.stringify(obj));
  return true;
}

function registerUser() {
  const username = els.registerUsername.value.trim();
  const password = els.registerPassword.value;
  if (!username || !password) {
    setStatus("请输入注册用户名和密码。");
    return;
  }
  if (!sendJson({
    type: "register",
    username,
    password
  })) {
    return;
  }
  setStatus("注册请求已发送。");
}

function loginUser() {
  const username = els.loginUsername.value.trim();
  const password = els.loginPassword.value;
  if (!username || !password) {
    setStatus("请输入登录用户名和密码。");
    return;
  }
  if (!sendJson({
    type: "login",
    username,
    password
  })) {
    return;
  }
  setStatus("登录请求已发送。");
}

function requestUserList() {
  sendJson({ type: "userlist_request" });
}

function requestHistory(peer) {
  if (!state.currentUser || !peer) {
    return;
  }
  sendJson({
    type: "history_request",
    user: state.currentUser,
    peer
  });
}

function sendChat() {
  const content = els.messageInput.value.trim();
  if (!content || !state.currentPeer || !state.currentUser) {
    return;
  }

  const message = {
    type: "chat",
    from: state.currentUser,
    to: state.currentPeer,
    message: content
  };

  if (!sendJson(message)) {
    return;
  }

  appendConversationMessage(state.currentPeer, {
    sender: state.currentUser,
    time: nowString(),
    content
  });

  els.messageInput.value = "";
  renderMessages();
  renderConversationList();
}

function handleServerMessage(data) {
  switch (data.type) {
    case "gateway_status":
      setStatus(data.message || "状态已更新。");
      break;
    case "register_reply":
      setStatus(data.message || "注册已处理。");
      break;
    case "login_reply":
      setStatus(data.message || "登录已处理。");
      if (data.status === "ok") {
        state.currentUser = els.loginUsername.value.trim();
        state.conversations = {};
        state.historyLoadedPeers = {};
        state.currentPeer = "";
        els.currentUserBadge.textContent = `当前用户：${state.currentUser}`;
        showMainView();
        renderConversationList();
        renderMessages();
        requestUserList();
      }
      break;
    case "userlist":
      state.onlineUsers = (data.users || []).filter((user) => user && user !== state.currentUser);
      renderOnlineUsers();
      break;
    case "chat":
      if (!data.from || !data.message) {
        return;
      }
      const peer = data.from === state.currentUser ? data.to : data.from;
      appendConversationMessage(peer, {
        sender: data.from,
        time: data.time || nowString(),
        content: data.message
      });
      if (!state.currentPeer) {
        state.currentPeer = peer;
      }
      renderConversationList();
      renderMessages();
      break;
    case "history_reply":
      handleHistoryReply(data);
      break;
    case "chat_reply":
      setStatus(data.message || "发送状态已更新。");
      break;
    default:
      if (data.message) {
        setStatus(data.message);
      }
  }
}

function appendConversationMessage(peer, message) {
  if (!state.conversations[peer]) {
    state.conversations[peer] = [];
  }
  state.conversations[peer].push(message);
}

function handleHistoryReply(data) {
  const peer = data.peer;
  if (!peer || data.status !== "ok") {
    if (data.message) {
      setStatus(data.message);
    }
    return;
  }

  const records = Array.isArray(data.records) ? data.records : [];
  state.conversations[peer] = records.map((item) => ({
    sender: item.sender,
    time: item.time || nowString(),
    content: item.content || ""
  }));
  state.historyLoadedPeers[peer] = true;
  renderConversationList();
  if (state.currentPeer === peer) {
    renderMessages();
  }
}

function renderOnlineUsers() {
  els.onlineUsers.innerHTML = "";
  if (state.onlineUsers.length === 0) {
    els.onlineUsers.innerHTML = '<div class="list-item"><div class="meta">当前没有其他在线用户。</div></div>';
    return;
  }

  state.onlineUsers.forEach((user) => {
    const item = document.createElement("button");
    item.className = "list-item";
    item.innerHTML = `<div class="name">${user}</div><div class="meta">双击或点击开始会话</div>`;
    item.addEventListener("click", () => openConversation(user));
    els.onlineUsers.appendChild(item);
  });
}

function renderConversationList() {
  els.conversationList.innerHTML = "";
  const peers = Object.keys(state.conversations);
  if (peers.length === 0) {
    els.conversationList.innerHTML = '<div class="list-item"><div class="meta">还没有聊天记录。</div></div>';
    return;
  }

  peers.forEach((peer) => {
    const item = document.createElement("button");
    item.className = `list-item${peer === state.currentPeer ? " active" : ""}`;
    const last = state.conversations[peer][state.conversations[peer].length - 1];
    item.innerHTML = `<div class="name">${peer}</div><div class="meta">${last.content.slice(0, 18)}</div>`;
    item.addEventListener("click", () => openConversation(peer));
    els.conversationList.appendChild(item);
  });
}

function openConversation(peer) {
  state.currentPeer = peer;
  if (!state.conversations[peer]) {
    state.conversations[peer] = [];
  }
  if (!state.historyLoadedPeers[peer]) {
    requestHistory(peer);
  }
  renderConversationList();
  renderMessages();
}

function renderMessages() {
  const peer = state.currentPeer;
  if (!peer) {
    els.chatTitle.textContent = "请选择一个联系人";
    els.chatSub.textContent = "";
    els.messages.innerHTML = '<div class="empty-state">当前还没有打开任何会话。登录后点击左侧在线用户，即可在浏览器里演示聊天。</div>';
    return;
  }

  els.chatTitle.textContent = `与 ${peer} 的聊天`;
  els.chatSub.textContent = "";

  const records = state.conversations[peer] || [];
  if (records.length === 0) {
    els.messages.innerHTML = '<div class="empty-state">会话已创建，但还没有消息。可以先发一条简短文本。</div>';
    return;
  }

  els.messages.innerHTML = records.map((item) => {
    const self = item.sender === state.currentUser;
    return `
      <article class="bubble${self ? " self" : ""}">
        <div class="head">
          <span>${escapeHtml(item.sender)}</span>
          <span>${escapeHtml(item.time)}</span>
        </div>
        <div class="content">${escapeHtml(item.content)}</div>
      </article>
    `;
  }).join("");
  els.messages.scrollTop = els.messages.scrollHeight;
}

function exportCurrentConversation() {
  if (!state.currentPeer) {
    setStatus("请先选择一个会话。");
    return;
  }

  const records = state.conversations[state.currentPeer] || [];
  const text = records
    .map((item) => `[${item.time}] ${item.sender}: ${item.content}`)
    .join("\n");
  const blob = new Blob([text], { type: "text/plain;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  link.href = url;
  link.download = `${state.currentUser || "user"}_${state.currentPeer}.txt`;
  link.click();
  URL.revokeObjectURL(url);
}

function escapeHtml(text) {
  return String(text)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#39;");
}

function showAuthView(type) {
  const loginActive = type === "login";
  els.showLoginBtn.classList.toggle("active", loginActive);
  els.showRegisterBtn.classList.toggle("active", !loginActive);
  els.loginCard.classList.toggle("active", loginActive);
  els.registerCard.classList.toggle("active", !loginActive);
}

function showMainView() {
  els.authShell.classList.add("hidden");
  els.mainShell.classList.remove("hidden");
}

function toggleSettingsPanel() {
  els.settingsPanel.classList.toggle("hidden");
}

function nowString() {
  const date = new Date();
  const pad = (value) => String(value).padStart(2, "0");
  return `${date.getFullYear()}-${pad(date.getMonth() + 1)}-${pad(date.getDate())} ${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`;
}

els.connectBtn.addEventListener("click", connectGateway);
els.toggleSettingsBtn.addEventListener("click", toggleSettingsPanel);
els.registerBtn.addEventListener("click", registerUser);
els.loginBtn.addEventListener("click", loginUser);
els.showLoginBtn.addEventListener("click", () => showAuthView("login"));
els.showRegisterBtn.addEventListener("click", () => showAuthView("register"));
els.refreshBtn.addEventListener("click", requestUserList);
els.exportBtn.addEventListener("click", exportCurrentConversation);
els.sendBtn.addEventListener("click", sendChat);

els.messageInput.addEventListener("keydown", (event) => {
  if (event.key === "Enter" && !event.shiftKey) {
    event.preventDefault();
    sendChat();
  }
});

renderOnlineUsers();
renderConversationList();
renderMessages();
showAuthView("login");
