import { Link } from 'react-router-dom';

interface CTAButtonProps {
  label: string;
  href: string;
  variant?: 'primary' | 'ghost';
  size?: 'sm' | 'md' | 'lg';
}

export function CTAButton({ label, href, variant = 'primary', size = 'md' }: CTAButtonProps) {
  const base = 'inline-flex items-center justify-center font-medium transition-all duration-200 focus:outline-none focus:ring-2 focus:ring-accent-primary/50 focus:ring-offset-2 focus:ring-offset-bg-primary';
  const sizes = {
    sm: 'px-4 py-2 text-sm',
    md: 'px-6 py-3 text-sm',
    lg: 'px-8 py-4 text-base',
  };
  const variants = {
    primary: 'bg-accent-primary text-bg-primary hover:bg-accent-primary/90 rounded',
    ghost: 'border border-border-active text-text-primary hover:bg-white/5 rounded',
  };

  const isExternal = href.startsWith('http');
  const className = `${base} ${sizes[size]} ${variants[variant]}`;

  if (isExternal) {
    return (
      <a href={href} className={className} target="_blank" rel="noopener noreferrer">
        {label}
      </a>
    );
  }

  return (
    <Link to={href} className={className}>
      {label}
    </Link>
  );
}
