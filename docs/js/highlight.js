// Nova Script — Lua Syntax Highlighter
// Tokenizer-based to handle nested patterns correctly

(function() {
  const KEYWORDS = new Set([
    'and','break','do','else','elseif','end','false','for','function',
    'goto','if','in','local','nil','not','or','repeat','return','then',
    'true','until','while'
  ]);

  const MODULES = new Set([
    'Timer','EventBus','World','Sound','Camera','Stats','Inventory',
    'Cooldown','Effect','Scene','SceneFX','Particles','Projectile',
    'EntityState','Pool','Nav','Trigger','Flag','Persist','Data','I18n',
    'Quest','Loot','Achievement','Conversation','Sequence','Notify',
    'Debug','Anim','Tween','Scheduler','InputEx','InputBind','Input',
    'Physics','Spatial','Math','Vec2','Random','Table','Easing',
    'StateMachine','Class','Log','Color','Game','Registry','Viewport',
    'Anim','String','coroutine','io','os','string','math','table'
  ]);

  function escape(s) {
    return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
  }

  function tokenize(code) {
    let result = '';
    let i = 0;
    const len = code.length;

    while (i < len) {
      // Long string [[ ... ]]
      if (code[i] === '[' && code[i+1] === '[') {
        const end = code.indexOf(']]', i+2);
        const s = end === -1 ? code.slice(i) : code.slice(i, end+2);
        result += '<span class="str">' + escape(s) + '</span>';
        i += s.length;
        continue;
      }
      // Comment --
      if (code[i] === '-' && code[i+1] === '-') {
        const nl = code.indexOf('\n', i);
        const s = nl === -1 ? code.slice(i) : code.slice(i, nl);
        result += '<span class="cmt">' + escape(s) + '</span>';
        i += s.length;
        continue;
      }
      // String "..."
      if (code[i] === '"') {
        let j = i+1;
        while (j < len && code[j] !== '"') { if (code[j] === '\\') j++; j++; }
        const s = code.slice(i, j+1);
        result += '<span class="str">' + escape(s) + '</span>';
        i += s.length;
        continue;
      }
      // String '...'
      if (code[i] === "'") {
        let j = i+1;
        while (j < len && code[j] !== "'") { if (code[j] === '\\') j++; j++; }
        const s = code.slice(i, j+1);
        result += '<span class="str">' + escape(s) + '</span>';
        i += s.length;
        continue;
      }
      // Number
      if (/[0-9]/.test(code[i]) || (code[i] === '.' && /[0-9]/.test(code[i+1]||''))) {
        let j = i;
        while (j < len && /[0-9a-fA-F_.xX]/.test(code[j])) j++;
        const s = code.slice(i, j);
        result += '<span class="num">' + escape(s) + '</span>';
        i = j;
        continue;
      }
      // Identifier / keyword / module
      if (/[a-zA-Z_]/.test(code[i])) {
        let j = i;
        while (j < len && /[a-zA-Z0-9_]/.test(code[j])) j++;
        const s = code.slice(i, j);
        if (KEYWORDS.has(s)) {
          result += '<span class="kw">' + s + '</span>';
        } else if (MODULES.has(s)) {
          result += '<span class="mod">' + s + '</span>';
        } else if (code[j] === '(') {
          result += '<span class="fn2">' + s + '</span>';
        } else {
          result += escape(s);
        }
        i = j;
        continue;
      }
      // Anything else (operators, punctuation, whitespace)
      result += escape(code[i]);
      i++;
    }
    return result;
  }

  function highlight() {
    document.querySelectorAll('pre code.lua').forEach(function(el) {
      el.innerHTML = tokenize(el.textContent);
    });
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', highlight);
  } else {
    highlight();
  }
})();
