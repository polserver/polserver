/*
 * Behaviour for the documentation pages. Everything here is an enhancement: the pages
 * are complete and navigable without it.
 *
 *  - search across all modules (the search field of the page header)
 *  - module pages: function filter, "current function" highlight, folding menus
 *    on narrow screens
 *  - index page: find a function in any module, collapse / expand all
 *
 * Loaded with "defer" from footer.inc, after searchindex.php (online) or
 * searchindex.js (offline), which set window.POL_DOCS_INDEX.
 */
(function () {
  'use strict';

  var script = document.currentScript;
  var offline = !!(script && script.getAttribute('data-offline'));
  var narrow = window.matchMedia('(max-width: 820px)');

  function $(id) { return document.getElementById(id); }
  function all(selector, root) {
    return Array.prototype.slice.call((root || document).querySelectorAll(selector));
  }
  function moduleUrl(file, func) {
    var url = offline ? file + '.html' : 'fullfunc.php?xmlfile=' + encodeURIComponent(file);
    return func ? url + '#' + encodeURIComponent(func) : url;
  }

  /* ---- search across all modules ---- */

  function initSearch() {
    var index = window.POL_DOCS_INDEX;
    var form = document.querySelector('#search-box form');
    var input = form && form.querySelector('input[type="search"], input[type="text"]');
    if (!index || !input) return;

    input.placeholder = 'Search the docs';
    input.setAttribute('aria-label', 'Search the docs');
    input.setAttribute('autocomplete', 'off');
    // the header of the main site offers a forum search here; on docs pages the
    // field searches the documentation instead
    all('label, button, input[type="image"], input[type="submit"]', form).forEach(function (el) {
      el.hidden = true;
    });

    var results = document.createElement('ul');
    results.className = 'doc-results';
    results.hidden = true;
    form.appendChild(results);

    function close() { results.hidden = true; }

    function search() {
      var q = input.value.trim().toLowerCase();
      results.textContent = '';
      if (!q) { close(); return; }

      // entry: [function name ('' for a module), module file, module name, description]
      var hits = [];
      index.forEach(function (entry) {
        var name = entry[0] || entry[2] + '.em';
        var at = name.toLowerCase().indexOf(q);
        if (at !== -1) hits.push({ entry: entry, name: name, rank: at === 0 ? 0 : 1 });
      });
      hits.sort(function (a, b) { return a.rank - b.rank || a.name.length - b.name.length; });

      hits.slice(0, 10).forEach(function (hit) {
        var entry = hit.entry;
        var li = document.createElement('li');
        var a = document.createElement('a');
        a.href = moduleUrl(entry[1], entry[0]);
        var name = document.createElement('span');
        name.className = 'rn';
        name.textContent = hit.name;
        var where = document.createElement('span');
        where.className = 'rm';
        where.textContent = (entry[0] ? entry[2] + '.em' : 'module') + (entry[3] ? ' · ' + entry[3] : '');
        a.appendChild(name);
        a.appendChild(where);
        li.appendChild(a);
        results.appendChild(li);
      });
      if (!hits.length) {
        var none = document.createElement('li');
        none.className = 'none';
        none.textContent = 'No function or module matches.';
        results.appendChild(none);
      }
      results.hidden = false;
    }

    input.addEventListener('input', search);
    input.addEventListener('focus', function () { if (input.value.trim()) search(); });
    input.addEventListener('keydown', function (e) { if (e.key === 'Escape') close(); });
    form.addEventListener('submit', function (e) {
      e.preventDefault();
      var first = results.querySelector('a');
      if (first && !results.hidden) window.location.href = first.href;
    });
    document.addEventListener('click', function (e) { if (!form.contains(e.target)) close(); });
    // a result on the page we are already on only changes the hash
    results.addEventListener('click', function (e) { if (e.target.closest('a')) close(); });
  }

  /* ---- module page ---- */

  function initModulePage() {
    var side = document.querySelector('.doc-side');
    if (!side) return;
    var menus = all('details.doc-menu', side);

    // function filter
    var filter = $('fn-filter');
    if (filter) {
      filter.addEventListener('input', function () {
        var q = filter.value.trim().toLowerCase();
        all('.fn-list li:not(.filter)', side).forEach(function (li) {
          li.hidden = q !== '' && li.textContent.toLowerCase().indexOf(q) === -1;
        });
      });
    }

    // wide screens: the function list is open beside the content;
    // narrow screens: both lists fold into a bar that stays at the top
    function applyMode() {
      menus.forEach(function (menu, i) { menu.open = !narrow.matches && i === 0; });
    }
    applyMode();
    narrow.addEventListener('change', applyMode);
    menus.forEach(function (menu) {
      menu.addEventListener('toggle', function () {
        if (!narrow.matches || !menu.open) return;
        menus.forEach(function (other) { if (other !== menu) other.open = false; });
      });
    });
    function closeMenus() { menus.forEach(function (menu) { menu.open = false; }); }
    side.addEventListener('click', function (e) {
      if (narrow.matches && e.target.closest('ul a')) closeMenus();
    });
    document.addEventListener('click', function (e) {
      if (narrow.matches && !side.contains(e.target)) closeMenus();
    });

    // highlight the function (module page) or section (guide) being read
    var links = {};
    all('.fn-list a, .guide-toc a', side).forEach(function (a) {
      links[decodeURIComponent(a.getAttribute('href').slice(1))] = a;
    });
    var functions = all('.doc-main .fn');
    if (!functions.length) {
      // guides: the targets of the table of contents, in page order
      functions = all('.doc-main [id], .doc-main a[name]').filter(function (el) {
        if (!el.id) el.id = el.getAttribute('name');
        return links.hasOwnProperty(el.id);
      });
    }
    var current = $('fn-current');
    var active = null;
    var ticking = false;
    function spy() {
      ticking = false;
      var line = narrow.matches ? 90 : 40;
      var found = null;
      for (var i = 0; i < functions.length; i++) {
        if (functions[i].getBoundingClientRect().top <= line) found = functions[i];
        else break;
      }
      if (found === active) return;
      if (active && links[active.id]) links[active.id].removeAttribute('aria-current');
      active = found;
      if (active && links[active.id]) {
        links[active.id].setAttribute('aria-current', 'true');
        // keep the highlighted name visible inside the scrolling list
        if (!narrow.matches && links[active.id].scrollIntoView)
          links[active.id].scrollIntoView({ block: 'nearest' });
      }
      if (current) current.textContent = active ? active.id : '';
    }
    window.addEventListener('scroll', function () {
      if (!ticking) { ticking = true; window.requestAnimationFrame(spy); }
    }, { passive: true });
    spy();
  }

  /* ---- index page ---- */

  function initIndexPage() {
    var filter = $('mod-filter');
    var modules = all('details.mod');
    if (!filter || !modules.length) return;
    var status = $('mod-status');
    var toggle = $('mod-toggle');

    filter.addEventListener('input', function () {
      var q = filter.value.trim().toLowerCase();
      var total = 0, inModules = 0;
      modules.forEach(function (module) {
        var count = 0;
        all('.fnames li', module).forEach(function (li) {
          var hit = !q || li.textContent.toLowerCase().indexOf(q) !== -1;
          li.hidden = !hit;
          if (hit) count++;
        });
        module.hidden = !!q && count === 0;
        if (q && count) { module.open = true; total += count; inModules++; }
        var whole = module.getAttribute('data-count');
        module.querySelector('.n').textContent = q ? count + ' of ' + whole : whole + (whole === '1' ? ' function' : ' functions');
      });
      if (status) {
        status.textContent = !q ? '' : total
          ? total + (total === 1 ? ' function in ' : ' functions in ') + inModules + (inModules === 1 ? ' module' : ' modules')
          : 'No function with that name in any module.';
      }
    });

    if (toggle) {
      toggle.addEventListener('click', function () {
        var collapse = toggle.textContent === 'Collapse all';
        modules.forEach(function (module) { module.open = !collapse; });
        toggle.textContent = collapse ? 'Expand all' : 'Collapse all';
      });
    }
  }

  initSearch();
  initModulePage();
  initIndexPage();
})();
